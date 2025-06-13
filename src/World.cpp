#include "World.hpp"
#include "Camera.hpp"
#include "EmbeddedShaders.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <glm/gtx/norm.hpp>
#include <thread>
#include <iostream>

// World constructor: Initializes renderers, generators, and starts all worker threads.
World::World()
{
    // Initialize the chunk renderer with a large buffer pool size.
    // Each pool can store a significant number of chunk meshes to reduce the frequency of creating new pools.
    // Vertex Buffer Capacity: 1,572,864 vertices -> ~54 MiB (with a vertex size of 36 bytes)
    // Index Buffer Capacity:  2,097,152 indices  -> ~4 MiB (with a 16-bit index size)
    m_chunkRenderer = std::make_unique<ChunkRenderer>(1572864, 2097152);
    m_terrainGenerator = std::make_unique<TerrainGenerator>(EmbeddedShaders::terrain_gen_comp);
    m_textureManager = std::make_unique<TextureManager>();
    m_lastPlayerChunkCoord = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
    
    // Load textures and populate the static block data map. Must be done after GL context is ready.
    m_textureManager->loadAndStitch();

    unsigned int numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
    for (unsigned int i = 0; i < numThreads; ++i)
    {
        m_workerThreads.emplace_back(&World::workerLoop, this);
    }
    m_managementThread = std::thread(&World::managementLoop, this);
}

// World destructor: Shuts down all threads and cleans up GPU resources.
World::~World()
{
    m_isShuttingDown = true;
    
    // Notify and join management thread first
    m_managementQueue.notify_all();
    if (m_managementThread.joinable()) {
        m_managementThread.join();
    }
    
    // Notify and join meshing worker threads
    m_meshRequestQueue.notify_all();
    for (auto &thread : m_workerThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    // Clean up any remaining in-flight jobs
    for (const auto &job : m_pendingGpuJobs)
    {
        glDeleteSync(job.fence);
    }
    for (const auto& job : m_pendingPboReads)
    {
        glDeleteSync(job.fence);
    }
}

// The main loop for the dedicated world management thread.
void World::managementLoop() {
    while (!m_isShuttingDown) {
        glm::ivec3 playerChunkCoord;
        // Wait for the main thread to send a new player position
        if (m_managementQueue.wait_and_pop(playerChunkCoord, m_isShuttingDown)) {
            updateChunkStates(playerChunkCoord);
        }
    }
}

// Heavy logic that determines which chunks to load/unload. Runs on the management thread.
void World::updateChunkStates(const glm::ivec3& playerChunkCoord) {
    const int renderDist = m_renderDistance;
    const long long unloadDistSq = (renderDist + 2) * (renderDist + 2);

    // --- Identify chunks to unload ---
    std::vector<glm::ivec3> coordsToUnload;
    {
        std::lock_guard<std::mutex> lock(m_worldDataMutex);
        for (const auto& [coord, chunk_ptr] : m_chunks) {
            long long dx = coord.x - playerChunkCoord.x;
            long long dy = coord.y - playerChunkCoord.y;
            long long dz = coord.z - playerChunkCoord.z;
            if (dx * dx + dy * dy + dz * dz > unloadDistSq) {
                coordsToUnload.push_back(coord);
            }
        }
    }
    for(const auto& coord : coordsToUnload) {
        m_unloadQueue.push(coord);
    }
    
    // --- Identify chunks to load ---
    std::vector<glm::ivec3> coordsToLoad;
    const int renderDistSq = renderDist * renderDist;

    for (int y = -renderDist; y <= renderDist; ++y) {
        for (int x = -renderDist; x <= renderDist; ++x) {
            for (int z = -renderDist; z <= renderDist; ++z) {
                if (x * x + y * y + z * z > renderDistSq) continue;
                
                glm::ivec3 coord = playerChunkCoord + glm::ivec3(x, y, z);

                std::lock_guard<std::mutex> lock(m_worldDataMutex);
                auto it = m_chunkStates.find(coord);

                if (it == m_chunkStates.end() || it->second == ChunkState::UNDEFINED) {
                    coordsToLoad.push_back(coord);
                    m_chunkStates[coord] = ChunkState::BACKLOG;
                }
            }
        }
    }

    if (!coordsToLoad.empty()) {
        std::sort(coordsToLoad.begin(), coordsToLoad.end(),
            [&](const glm::ivec3& a, const glm::ivec3& b) {
                return glm::distance2(glm::vec3(a), glm::vec3(playerChunkCoord)) <
                    glm::distance2(glm::vec3(b), glm::vec3(playerChunkCoord));
            });
            
        for (const auto& coord : coordsToLoad) {
            m_gpuRequestQueue.push(coord);
        }
    }
}


// The main loop for each CPU meshing worker thread.
void World::workerLoop()
{
    while (!m_isShuttingDown)
    {
        glm::ivec3 coord;
        if (m_meshRequestQueue.wait_and_pop(coord, m_isShuttingDown))
        {
            std::shared_ptr<Chunk> chunkToMesh;
            {
                std::lock_guard<std::mutex> lock(m_worldDataMutex);
                auto it = m_chunks.find(coord);
                if (it != m_chunks.end())
                {
                    chunkToMesh = it->second;
                }
            }
            if (chunkToMesh)
            {
                // Generate the mesh for the chunk (a computationally expensive operation) and push the result.
                m_meshResultQueue.push(chunkToMesh->generateMeshStandalone(*this));
            }
        }
    }
}

// Main world update function, called every frame on the main thread.
void World::update(const glm::vec3 &playerPos)
{
    glm::ivec3 playerChunkCoord = {
        static_cast<int>(std::floor(playerPos.x / Constants::CHUNK_WIDTH)),
        static_cast<int>(std::floor(playerPos.y / Constants::CHUNK_WIDTH)),
        static_cast<int>(std::floor(playerPos.z / Constants::CHUNK_WIDTH))};

    // --- Trigger world management if player has moved ---
    if (playerChunkCoord != m_lastPlayerChunkCoord)
    {
        m_lastPlayerChunkCoord = playerChunkCoord;
        m_managementQueue.push(playerChunkCoord);
    }

    // --- Process all asynchronous pipeline stages on the main thread ---
    processUnloads();
    processPboReads();
    processCompletedGpuJobs();
    dispatchGpuJobs();

    // --- Finalize meshes that have been completed by worker threads ---
    MeshResult result;
    while (m_meshResultQueue.try_pop(result))
    {
        std::lock_guard<std::mutex> lock(m_worldDataMutex);
        auto it = m_chunks.find(result.chunkCoord);
        if (it != m_chunks.end())
        {
            m_chunkRenderer->freeMesh(it->second->getOpaqueMeshAllocation());
            m_chunkRenderer->freeMesh(it->second->getTransparentMeshAllocation());

            it->second->setOpaqueMeshAllocation(m_chunkRenderer->allocateMesh(result.opaqueVertices, result.opaqueIndices));
            it->second->setTransparentMeshAllocation(m_chunkRenderer->allocateMesh(result.transparentVertices, result.transparentIndices));
            
            m_chunkStates[result.chunkCoord] = ChunkState::READY;
        }
    }
}

// Processes the unload queue on the main thread (needs OpenGL context).
void World::processUnloads() {
    glm::ivec3 coord;
    while(m_unloadQueue.try_pop(coord)) {
        std::lock_guard<std::mutex> lock(m_worldDataMutex);
        auto it = m_chunks.find(coord);
        if (it != m_chunks.end()) {
            m_chunkRenderer->freeMesh(it->second->getOpaqueMeshAllocation());
            m_chunkRenderer->freeMesh(it->second->getTransparentMeshAllocation());
            m_chunks.erase(it);
        }
        m_chunkStates[coord] = ChunkState::UNDEFINED;
    }
}


// Tries to send jobs from the request queue to the GPU.
void World::dispatchGpuJobs()
{
    glm::ivec3 coord;
    // Check if there are slots and if there's anything in the queue
    if (m_terrainGenerator->hasAvailableJobSlots() && m_gpuRequestQueue.try_pop(coord))
    {
        auto job = m_terrainGenerator->dispatchJob(coord);
        if (job)
        {
            m_pendingGpuJobs.push_back(*job);
            std::lock_guard<std::mutex> lock(m_worldDataMutex);
            m_chunkStates[coord] = ChunkState::GPU_PENDING;
        } else {
             // If dispatchJob fails unexpectedly (e.g., in a multi-threaded context where another
             // thread took the last slot), the request is effectively dropped for this frame.
             // The management thread will re-queue it on a subsequent update if it is still needed.
        }
    }
}

// Checks for finished GPU jobs and schedules them for async read-back via PBOs.
void World::processCompletedGpuJobs()
{
    for (auto it = m_pendingGpuJobs.begin(); it != m_pendingGpuJobs.end();)
    {
        GLenum waitResult = glClientWaitSync(it->fence, 0, 0);
        if (waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED)
        {
            auto pboJob = m_terrainGenerator->scheduleRead(*it);

            if (pboJob) {
                m_pendingPboReads.push_back(*pboJob);
                {
                    std::lock_guard<std::mutex> lock(m_worldDataMutex);
                    m_chunkStates[it->chunkCoord] = ChunkState::PBO_PENDING;
                }
                m_terrainGenerator->releaseGpuJob(*it);
                it = m_pendingGpuJobs.erase(it);
            } else {
                // PBO pool was full, try again next frame.
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

// Checks for finished PBO-to-RAM transfers, creates Chunk objects, and queues them for meshing.
void World::processPboReads()
{
    for (auto it = m_pendingPboReads.begin(); it != m_pendingPboReads.end();)
    {
        GLenum waitResult = glClientWaitSync(it->fence, 0, 0);
        if (waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED)
        {
            uint32_t blockData[Constants::CHUNK_VOL];
            m_terrainGenerator->readPboData(*it, blockData);

            {
                std::lock_guard<std::mutex> lock(m_worldDataMutex);
                m_chunks[it->chunkCoord] = std::make_shared<Chunk>(it->chunkCoord, blockData);
                m_chunkStates[it->chunkCoord] = ChunkState::DATA_READY;
            }
            
            m_meshRequestQueue.push(it->chunkCoord);
             {
                std::lock_guard<std::mutex> lock(m_worldDataMutex);
                m_chunkStates[it->chunkCoord] = ChunkState::MESH_PENDING;
            }
            
            m_terrainGenerator->releasePboJob(*it);
            it = m_pendingPboReads.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// Gets the block type at a given world position (thread-safe).
BlockType World::getBlock(const glm::ivec3 &worldBlockPos) const
{
    glm::ivec3 chunkCoord(
        static_cast<int>(std::floor(static_cast<float>(worldBlockPos.x) / Constants::CHUNK_WIDTH)),
        static_cast<int>(std::floor(static_cast<float>(worldBlockPos.y) / Constants::CHUNK_WIDTH)),
        static_cast<int>(std::floor(static_cast<float>(worldBlockPos.z) / Constants::CHUNK_WIDTH))
    );

    std::shared_ptr<Chunk> chunk;
    {
        std::lock_guard<std::mutex> lock(m_worldDataMutex);
        auto it = m_chunks.find(chunkCoord);
        if (it == m_chunks.end())
        {
            return BlockType::AIR;
        }
        chunk = it->second;
    }

    glm::ivec3 localPos = worldBlockPos - (chunkCoord * Constants::CHUNK_DIM);
    return chunk->getBlock(localPos.x, localPos.y, localPos.z);
}

// Renders all visible chunks.
void World::render(Shader &shader, const Camera &camera)
{
    // Bind the texture atlas to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureManager->getAtlasID());
    // Tell the shader that our "u_textureAtlas" uniform should use texture unit 0
    shader.setInt("u_textureAtlas", 0);

    std::vector<std::shared_ptr<Chunk>> chunksToRender;
    {
        std::lock_guard<std::mutex> lock(m_worldDataMutex);
        chunksToRender.reserve(m_chunks.size());
        for (auto const &[coord, chunk] : m_chunks)
        {
            // Frustum cull here before adding to render list
            if (camera.isAABBVisible(chunk->getExpandedAABB().min, chunk->getExpandedAABB().max))
            {
                chunksToRender.push_back(chunk);
            }
        }
    }

    if (chunksToRender.empty())
        return;

    // --- Opaque Pass ---
    glDisable(GL_BLEND);                          // Opaque objects don't need blending
    glDepthMask(GL_TRUE);                         // Ensure depth writing is on
    shader.setBool("u_isTransparentPass", false); // Inform shader this is the opaque pass

    // Sort chunks by pool index to minimize VAO binds.
    std::sort(chunksToRender.begin(), chunksToRender.end(),
              [](const std::shared_ptr<Chunk> &a, const std::shared_ptr<Chunk> &b)
              {
                  return a->getOpaqueMeshAllocation().poolIndex < b->getOpaqueMeshAllocation().poolIndex;
              });

    for (const auto &chunk : chunksToRender)
    {
        const auto &alloc = chunk->getOpaqueMeshAllocation();
        if (alloc.isValid())
        {
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), chunk->getPosition());
            shader.setMat4("ModelMatrix", ModelMatrix);
            m_chunkRenderer->draw(alloc);
        }
    }

    // --- Transparent Pass ---
    glEnable(GL_BLEND); 
    glDepthMask(GL_FALSE);                       
    shader.setBool("u_isTransparentPass", true); 

    std::vector<const Chunk *> transparentChunks;
    transparentChunks.reserve(chunksToRender.size());
    for (const auto &chunk : chunksToRender)
    {
        if (chunk->getTransparentMeshAllocation().isValid())
        {
            transparentChunks.push_back(chunk.get());
        }
    }

    if (!transparentChunks.empty())
    {
        const glm::vec3 cameraPos = camera.getPosition();
        std::sort(transparentChunks.begin(), transparentChunks.end(),
                  [&cameraPos](const Chunk *a, const Chunk *b)
                  {
                      return glm::distance2(a->getCenterPosition(), cameraPos) > glm::distance2(b->getCenterPosition(), cameraPos);
                  });

        std::stable_sort(transparentChunks.begin(), transparentChunks.end(),
                         [](const Chunk *a, const Chunk *b)
                         {
                             return a->getTransparentMeshAllocation().poolIndex < b->getTransparentMeshAllocation().poolIndex;
                         });

        for (const auto *chunk : transparentChunks)
        {
            const auto &alloc = chunk->getTransparentMeshAllocation();
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), chunk->getPosition());
            shader.setMat4("ModelMatrix", ModelMatrix);
            m_chunkRenderer->draw(alloc);
        }
    }

    glDepthMask(GL_TRUE); 
}

glm::vec2 World::getAtlasNormalizedTileSize() const {
    if (m_textureManager) {
        return m_textureManager->getNormalizedTileSize();
    }
    std::cerr << "Error: TextureManager not initialized in World when getAtlasNormalizedTileSize was called." << std::endl;
    return glm::vec2(0.0f); // Should not happen if constructor order is correct
}
