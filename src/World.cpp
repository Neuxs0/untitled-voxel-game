#include "World.hpp"
#include "Camera.hpp"
#include "EmbeddedShaders.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <glm/gtx/norm.hpp>
#include <thread>
#include <iostream>

// World constructor: Initializes renderers, generators, and starts worker threads.
World::World()
{
    m_chunkRenderer = std::make_unique<ChunkRenderer>(2097152, 8388608); // 2M vertices, 8M indices
    m_terrainGenerator = std::make_unique<TerrainGenerator>(EmbeddedShaders::terrain_gen_comp);
    m_lastPlayerChunkCoord = {std::numeric_limits<int>::max(), 0, std::numeric_limits<int>::max()};
    
    unsigned int numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
    for (unsigned int i = 0; i < numThreads; ++i)
    {
        m_workerThreads.emplace_back(&World::workerLoop, this);
    }
}

// World destructor: Shuts down threads and cleans up GPU resources.
World::~World()
{
    m_isShuttingDown = true;
    m_meshRequestQueue.notify_all(); // Wake up any sleeping worker threads so they can exit
    for (auto &thread : m_workerThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    for (const auto &job : m_pendingGpuJobs)
    {
        glDeleteSync(job.fence);
    }
}

// The main loop for each CPU worker thread.
void World::workerLoop()
{
    while (!m_isShuttingDown)
    {
        glm::ivec3 coord;
        if (m_meshRequestQueue.wait_and_pop(coord, m_isShuttingDown))
        {
            std::shared_ptr<Chunk> chunkToMesh;
            {
                std::lock_guard<std::mutex> lock(m_chunksMutex);
                auto it = m_chunks.find(coord);
                if (it != m_chunks.end())
                {
                    chunkToMesh = it->second;
                }
            }
            if (chunkToMesh)
            {
                // The expensive greedy meshing happens here on a background thread.
                m_meshResultQueue.push(chunkToMesh->generateMeshStandalone(*this));
            }
        }
    }
}

// Main world update function, called every frame.
void World::update(const glm::vec3 &playerPos)
{
    glm::ivec3 playerChunkCoord = {
        static_cast<int>(std::floor(playerPos.x / Constants::CHUNK_WIDTH)), 0,
        static_cast<int>(std::floor(playerPos.z / Constants::CHUNK_WIDTH))};

    // --- Update chunk lists if player has moved ---
    if (playerChunkCoord != m_lastPlayerChunkCoord)
    {
        m_lastPlayerChunkCoord = playerChunkCoord;
        updateChunkLists();
    }

    // --- Process asynchronous results ---
    processCompletedGpuJobs(); // Check for finished block data from the GPU
    dispatchGpuJobs();         // Send new generation jobs to the GPU from the backlog

    // --- Dispatch work to CPU meshing threads ---
    int dispatched = 0;
    while (!m_meshingQueue.empty() && dispatched < 4) // Dispatch up to 4 chunks per frame
    {
        m_meshRequestQueue.push(m_meshingQueue.front());
        m_meshingQueue.pop_front();
        dispatched++;
    }

    // --- Finalize meshes on the main thread ---
    MeshResult result;
    while (m_meshResultQueue.try_pop(result))
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        auto it = m_chunks.find(result.chunkCoord);
        if (it != m_chunks.end())
        {
            // Free old mesh from the pool before allocating a new one
            m_chunkRenderer->freeMesh(it->second->getOpaqueMeshAllocation());
            m_chunkRenderer->freeMesh(it->second->getTransparentMeshAllocation());

            // Allocate the new optimized mesh
            it->second->setOpaqueMeshAllocation(m_chunkRenderer->allocateMesh(result.opaqueVertices, result.opaqueIndices));
            it->second->setTransparentMeshAllocation(m_chunkRenderer->allocateMesh(result.transparentVertices, result.transparentIndices));
        }
    }
}

// Identifies chunks to load/unload and prioritizes them.
void World::updateChunkLists()
{
    std::vector<glm::ivec3> chunksToUnload;
    std::vector<glm::ivec3> newChunksRequired;
    const int renderDist = m_renderDistance;
    const long long unloadDistSq = (renderDist + 2) * (renderDist + 2);

    // Identify chunks to unload
    std::lock_guard<std::mutex> lock(m_chunksMutex);
    for (const auto& [coord, chunk] : m_chunks) {
        long long dx = coord.x - m_lastPlayerChunkCoord.x;
        long long dz = coord.z - m_lastPlayerChunkCoord.z;
        if (dx*dx + dz*dz > unloadDistSq) {
            chunksToUnload.push_back(coord);
        }
    }

    // Unload them
    for (const auto& coord : chunksToUnload) {
        auto it = m_chunks.find(coord);
        if (it != m_chunks.end()) {
            m_chunkRenderer->freeMesh(it->second->getOpaqueMeshAllocation());
            m_chunkRenderer->freeMesh(it->second->getTransparentMeshAllocation());
            m_chunks.erase(it);
        }
    }

    // Identify chunks to load
    for (int x = -renderDist; x <= renderDist; ++x) {
        for (int z = -renderDist; z <= renderDist; ++z) {
            if (x*x + z*z > renderDist*renderDist) continue;
            
            glm::ivec3 coord = m_lastPlayerChunkCoord + glm::ivec3(x, 0, z);

            if (m_chunks.count(coord)) continue;

            bool isPending = false;
            for(const auto& job : m_pendingGpuJobs) if(job.chunkCoord == coord) { isPending=true; break; }
            if(isPending) continue;
            
            bool inBacklog = false;
            for(const auto& backlogCoord : m_generationBacklog) if(backlogCoord == coord) { inBacklog=true; break; }
            if(inBacklog) continue;
            
            newChunksRequired.push_back(coord);
        }
    }

    // Sort new chunks by distance to player
    std::sort(newChunksRequired.begin(), newChunksRequired.end(),
        [&](const glm::ivec3& a, const glm::ivec3& b) {
            return glm::distance2(glm::vec2(a.x, a.z), glm::vec2(m_lastPlayerChunkCoord.x, m_lastPlayerChunkCoord.z)) <
                   glm::distance2(glm::vec2(b.x, b.z), glm::vec2(m_lastPlayerChunkCoord.x, m_lastPlayerChunkCoord.z));
        });

    // Add sorted chunks to the main generation backlog
    m_generationBacklog.insert(m_generationBacklog.end(), newChunksRequired.begin(), newChunksRequired.end());
}

// Tries to send jobs from the backlog to the GPU.
void World::dispatchGpuJobs()
{
    while (!m_generationBacklog.empty())
    {
        auto job = m_terrainGenerator->dispatchJob(m_generationBacklog.front());
        if (job)
        {
            m_pendingGpuJobs.push_back(*job);
            m_generationBacklog.pop_front();
        }
        else
        {
            // GPU job pool is full, try again next frame
            break; 
        }
    }
}

// Checks for finished GPU jobs, creates Chunk objects, and queues them for meshing.
void World::processCompletedGpuJobs()
{
    bool needsResort = false;
    for (auto it = m_pendingGpuJobs.begin(); it != m_pendingGpuJobs.end();)
    {
        GLenum waitResult = glClientWaitSync(it->fence, 0, 0);
        if (waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED)
        {
            uint32_t blockData[Constants::CHUNK_VOL];
            m_terrainGenerator->readJobResults(*it, blockData);

            {
                std::lock_guard<std::mutex> lock(m_chunksMutex);
                m_chunks[it->chunkCoord] = std::make_shared<Chunk>(it->chunkCoord, blockData);
            }
            
            // This chunk is now ready to be meshed by a CPU thread
            m_meshingQueue.push_back(it->chunkCoord);
            needsResort = true;
            
            m_terrainGenerator->releaseJobResources(*it);
            it = m_pendingGpuJobs.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (needsResort)
    {
        // Sort the meshing queue by distance so CPU workers prioritize chunks near the player
        std::sort(m_meshingQueue.begin(), m_meshingQueue.end(),
            [&](const glm::ivec3& a, const glm::ivec3& b) {
                return glm::distance2(glm::vec2(a.x, a.z), glm::vec2(m_lastPlayerChunkCoord.x, m_lastPlayerChunkCoord.z)) <
                       glm::distance2(glm::vec2(b.x, b.z), glm::vec2(m_lastPlayerChunkCoord.x, m_lastPlayerChunkCoord.z));
            });
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
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        auto it = m_chunks.find(chunkCoord);
        if (it == m_chunks.end())
        {
            // If the chunk doesn't exist, we can't get a block from it.
            // This could be extended to synchronously generate data if needed.
            return BlockType::AIR;
        }
        chunk = it->second;
    }

    // This part does not need a lock because the chunk's block data is immutable after creation.
    glm::ivec3 localPos = worldBlockPos - (chunkCoord * Constants::CHUNK_DIM);
    return chunk->getBlock(localPos.x, localPos.y, localPos.z);
}


// Renders all visible chunks.
void World::render(Shader &shader, const Camera &camera)
{
    m_chunkRenderer->bind();

    std::vector<std::shared_ptr<Chunk>> chunksToRender;
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        chunksToRender.reserve(m_chunks.size());
        for (auto const &[coord, chunk] : m_chunks)
        {
            chunksToRender.push_back(chunk);
        }
    }

    // Opaque pass
    for (const auto &chunk : chunksToRender)
    {
        if (camera.isAABBVisible(chunk->getExpandedAABB().min, chunk->getExpandedAABB().max))
        {
            const auto &alloc = chunk->getOpaqueMeshAllocation();
            if (alloc.isValid())
            {
                glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), chunk->getPosition());
                shader.setMat4("ModelMatrix", ModelMatrix);
                ChunkRenderer::draw(alloc);
            }
        }
    }

    // Transparent pass
    glDepthMask(GL_FALSE);
    std::vector<const Chunk *> transparentChunks;
    for (const auto &chunk : chunksToRender) {
        if (chunk->getTransparentMeshAllocation().isValid() && camera.isAABBVisible(chunk->getExpandedAABB().min, chunk->getExpandedAABB().max)) {
            transparentChunks.push_back(chunk.get());
        }
    }

    if (!transparentChunks.empty()) {
        const glm::vec3 cameraPos = camera.getPosition();
        std::sort(transparentChunks.begin(), transparentChunks.end(),
            [&cameraPos](const Chunk *a, const Chunk *b) {
                return glm::distance2(a->getPosition(), cameraPos) > glm::distance2(b->getPosition(), cameraPos);
            });

        for (const auto* chunk : transparentChunks) {
            const auto &alloc = chunk->getTransparentMeshAllocation();
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), chunk->getPosition());
            shader.setMat4("ModelMatrix", ModelMatrix);
            ChunkRenderer::draw(alloc);
        }
    }

    glDepthMask(GL_TRUE);
    m_chunkRenderer->unbind();
}
