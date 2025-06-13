#pragma once

#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp> // Required for glm::vec2
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include <deque>
#include <unordered_map>

#include "Chunk.hpp"
#include "Shader.hpp"
#include "Block.hpp"
#include "TerrainGenerator.hpp"
#include "ChunkRenderer.hpp"
#include "ThreadSafeQueue.hpp"
#include "Constants.hpp"
#include "TextureManager.hpp" 

class Camera;

// Custom comparator for glm::ivec3 to allow its use as a key in std::map.
struct ivec3_comp
{
    bool operator()(const glm::ivec3 &a, const glm::ivec3 &b) const
    {
        return std::lexicographical_compare(glm::value_ptr(a), glm::value_ptr(a) + 3, glm::value_ptr(b), glm::value_ptr(b) + 3);
    }
};

// Custom hash function for glm::ivec3 to use in unordered_map.
struct ivec3_hash {
    std::size_t operator()(const glm::ivec3& v) const {
        std::size_t seed = 0;
        std::hash<int> hasher;
        seed ^= hasher(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

// The lifecycle state of a chunk.
enum class ChunkState {
    UNDEFINED,            // Not yet processed, or unloaded
    BACKLOG,              // In the queue for GPU generation
    GPU_PENDING,          // Compute job sent to GPU
    PBO_PENDING,          // PBO readback scheduled
    DATA_READY,           // Data is ready, needs meshing
    MESH_PENDING,         // Sent to CPU worker for meshing
    READY                 // Meshed and ready to be rendered
};


class World
{
private:
    std::map<glm::ivec3, std::shared_ptr<Chunk>, ivec3_comp> m_chunks;
    std::unordered_map<glm::ivec3, ChunkState, ivec3_hash> m_chunkStates;
    mutable std::mutex m_worldDataMutex;

    // The player's render distance, in chunks.
    int m_renderDistance = Constants::RENDER_DISTANCE;
    glm::ivec3 m_lastPlayerChunkCoord;

    std::unique_ptr<ChunkRenderer> m_chunkRenderer;
    std::unique_ptr<TerrainGenerator> m_terrainGenerator;
    std::unique_ptr<TextureManager> m_textureManager; 

    // --- GPU Job Management ---
    std::list<GpuJob> m_pendingGpuJobs;
    std::list<PboReadJob> m_pendingPboReads;

    // --- World Management Thread ---
    std::thread m_managementThread;
    ThreadSafeQueue<glm::ivec3> m_managementQueue; 
    ThreadSafeQueue<glm::ivec3> m_gpuRequestQueue; 
    ThreadSafeQueue<glm::ivec3> m_unloadQueue;     

    // --- CPU Meshing Pipeline ---
    std::vector<std::thread> m_workerThreads;
    std::atomic<bool> m_isShuttingDown{false};
    ThreadSafeQueue<MeshResult> m_meshResultQueue;
    ThreadSafeQueue<glm::ivec3> m_meshRequestQueue;

    // --- Private Helper Functions ---
    void managementLoop();
    void updateChunkStates(const glm::ivec3& playerChunkCoord);

    void processUnloads();
    void dispatchGpuJobs();
    void processCompletedGpuJobs();
    void processPboReads();
    void workerLoop();

public:
    World();
    ~World();
    void update(const glm::vec3 &playerPos);
    void render(Shader &shader, const Camera &camera);
    BlockType getBlock(const glm::ivec3 &worldBlockPos) const;
    glm::vec2 getAtlasNormalizedTileSize() const;
};
