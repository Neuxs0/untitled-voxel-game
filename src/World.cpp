#include "World.hpp"
#include <algorithm>

// World constructor.
World::World()
{
    // Chunks are loaded dynamically in the update() function.
}

// Update the world state based on the player's position.
void World::update(const glm::vec3 &playerPos)
{
    glm::ivec3 playerChunkCoord = {
        static_cast<int>(std::floor(playerPos.x / Constants::CHUNK_WIDTH)),
        0,
        static_cast<int>(std::floor(playerPos.z / Constants::CHUNK_WIDTH))};

    const int renderDistSquared = m_renderDistance * m_renderDistance;
    std::vector<glm::ivec3> chunksToCreate;

    for (int x = -m_renderDistance; x <= m_renderDistance; ++x)
    {
        for (int z = -m_renderDistance; z <= m_renderDistance; ++z)
        {
            if (x * x + z * z <= renderDistSquared)
            {
                glm::ivec3 coord = {playerChunkCoord.x + x, 0, playerChunkCoord.z + z};
                if (m_chunks.find(coord) == m_chunks.end())
                {
                    chunksToCreate.push_back(coord);
                }
            }
        }
    }

    if (!chunksToCreate.empty())
    {
        std::set<glm::ivec3, ivec3_comp> chunksToQueue;

        for (const auto &coord : chunksToCreate)
        {
            m_chunks[coord] = std::make_unique<Chunk>(coord);
            chunksToQueue.insert(coord);
        }

        const glm::ivec3 neighborOffsets[] = {
            {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}};

        for (const auto &newCoord : chunksToCreate)
        {
            for (const auto &offset : neighborOffsets)
            {
                glm::ivec3 neighborCoord = newCoord + offset;
                if (m_chunks.count(neighborCoord))
                {
                    chunksToQueue.insert(neighborCoord);
                }
            }
        }

        for (const auto &coord : chunksToQueue)
        {
            m_meshingQueue.push_back(coord);
        }
    }

    unloadDistantChunks(playerChunkCoord);

    int chunksProcessed = 0;
    while (!m_meshingQueue.empty() && chunksProcessed < MESHING_RATE)
    {
        glm::ivec3 coord = m_meshingQueue.front();
        m_meshingQueue.pop_front();

        auto it = m_chunks.find(coord);
        if (it != m_chunks.end())
        {
            it->second->generateMesh(*this);
        }

        chunksProcessed++;
    }
}

// Unload chunks that are too far from the player.
void World::unloadDistantChunks(const glm::ivec3 &playerChunkCoord)
{
    std::vector<glm::ivec3> chunksToUnload;
    const int unloadDist = m_renderDistance + 1;
    const int unloadDistSquared = unloadDist * unloadDist;

    for (auto const &[coord, chunk] : m_chunks)
    {
        int distX = coord.x - playerChunkCoord.x;
        int distZ = coord.z - playerChunkCoord.z;

        if (distX * distX + distZ * distZ > unloadDistSquared)
        {
            chunksToUnload.push_back(coord);
        }
    }

    for (const auto &coord : chunksToUnload)
    {
        m_meshingQueue.erase(
            std::remove(m_meshingQueue.begin(), m_meshingQueue.end(), coord),
            m_meshingQueue.end());
        m_chunks.erase(coord);
    }
}

// Get the block at a given world position.
BlockType World::getBlock(const glm::ivec3 &worldBlockPos) const
{
    const int D = Constants::CHUNK_DIM;
    glm::ivec3 chunkCoord;

    chunkCoord.x = (worldBlockPos.x >= 0) ? (worldBlockPos.x / D) : ((worldBlockPos.x - D + 1) / D);
    chunkCoord.y = (worldBlockPos.y >= 0) ? (worldBlockPos.y / D) : ((worldBlockPos.y - D + 1) / D);
    chunkCoord.z = (worldBlockPos.z >= 0) ? (worldBlockPos.z / D) : ((worldBlockPos.z - D + 1) / D);

    auto it = m_chunks.find(chunkCoord);
    if (it == m_chunks.end())
    {
        return BlockType::AIR;
    }

    int localX = worldBlockPos.x - chunkCoord.x * D;
    int localY = worldBlockPos.y - chunkCoord.y * D;
    int localZ = worldBlockPos.z - chunkCoord.z * D;

    return it->second->getBlock(localX, localY, localZ);
}

// Render the world.
void World::render(Shader &shader)
{
    // Render opaque blocks first
    for (auto const &[coord, chunk] : m_chunks)
    {
        chunk->renderOpaque(shader);
    }
    // Then render transparent blocks
    glDepthMask(GL_FALSE);
    for (auto const &[coord, chunk] : m_chunks)
    {
        chunk->renderTransparent(shader);
    }
    glDepthMask(GL_TRUE);
}
