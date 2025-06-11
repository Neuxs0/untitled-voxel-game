#pragma once

#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <deque>
#include <set>

#include "Chunk.hpp"
#include "Shader.hpp"
#include "Block.hpp"

// Custom comparator to allow using glm::ivec3 as a key in std::map.
struct ivec3_comp
{
    bool operator()(const glm::ivec3 &a, const glm::ivec3 &b) const
    {
        return std::lexicographical_compare(glm::value_ptr(a), glm::value_ptr(a) + 3,
                                            glm::value_ptr(b), glm::value_ptr(b) + 3);
    }
};

// Represents the world, which is made up of chunks.
class World
{
private:
    std::map<glm::ivec3, std::unique_ptr<Chunk>, ivec3_comp> m_chunks;
    int m_renderDistance = Constants::RENDER_DISTANCE;
    // A queue of chunks to be meshed.
    std::deque<glm::ivec3> m_meshingQueue;
    // The number of chunks to mesh per frame.
    static constexpr int MESHING_RATE = 4;

    // Load a chunk at the given coordinate.
    void loadChunk(const glm::ivec3 &chunkCoord);
    // Unload chunks that are too far from the player.
    void unloadDistantChunks(const glm::ivec3 &playerChunkCoord);

public:
    // Constructor.
    World();

    // Update the world state based on the player's position.
    void update(const glm::vec3 &playerPos);
    // Get the block at a given world position.
    BlockType getBlock(const glm::ivec3 &worldBlockPos) const;
    // Render the world.
    void render(Shader &shader);
};
