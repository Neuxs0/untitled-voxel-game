#pragma once

#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include <deque>

#include "Chunk.hpp"
#include "Shader.hpp"
#include "Block.hpp"
#include "TerrainGenerator.hpp"
#include "ChunkRenderer.hpp"
#include "ThreadSafeQueue.hpp"
#include "Constants.hpp"

class Camera;

// Custom comparator for glm::ivec3 to allow its use as a key in std::map.
struct ivec3_comp
{
    bool operator()(const glm::ivec3 &a, const glm::ivec3 &b) const
    {
        return std::lexicographical_compare(glm::value_ptr(a), glm::value_ptr(a) + 3, glm::value_ptr(b), glm::value_ptr(b) + 3);
    }
};

class World
{
private:
    std::map<glm::ivec3, std::shared_ptr<Chunk>, ivec3_comp> m_chunks;
    mutable std::mutex m_chunksMutex;

    // The player's render distance, in chunks.
    int m_renderDistance = Constants::RENDER_DISTANCE;

    glm::ivec3 m_lastPlayerChunkCoord;

    std::unique_ptr<ChunkRenderer> m_chunkRenderer;
    std::unique_ptr<TerrainGenerator> m_terrainGenerator;

    // --- GPU Job Management ---
    std::list<glm::ivec3> m_generationBacklog;
    std::list<GpuJob> m_pendingGpuJobs;

    // --- CPU Meshing Pipeline ---
    std::deque<glm::ivec3> m_meshingQueue;
    std::vector<std::thread> m_workerThreads;
    std::atomic<bool> m_isShuttingDown{false};
    ThreadSafeQueue<MeshResult> m_meshResultQueue;
    ThreadSafeQueue<glm::ivec3> m_meshRequestQueue;

    // --- Private Helper Functions ---
    void updateChunkLists();
    void dispatchGpuJobs();
    void processCompletedGpuJobs();
    void workerLoop();

public:
    World();
    ~World();
    void update(const glm::vec3 &playerPos);
    void render(Shader &shader, const Camera &camera);
    BlockType getBlock(const glm::ivec3 &worldBlockPos) const;
};
