#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string_view>
#include <cstdint>
#include <vector>
#include <deque>
#include <optional>

// A structure to track an in-flight GPU generation job.
struct GpuJob
{
    glm::ivec3 chunkCoord;
    GLuint ssbo;
    GLsync fence;
};

// Manages the GPU-side terrain data generation using a compute shader.
class TerrainGenerator
{
private:
    GLuint m_computeProgramID;

    // A pool of buffers to allow multiple jobs to be in-flight simultaneously.
    std::vector<GLuint> m_ssboPool;
    std::deque<GLuint> m_freeSsboQueue;

    // The maximum number of chunk generations that can be in-flight on the GPU.
    static constexpr int MAX_CONCURRENT_JOBS = 64;

public:
    TerrainGenerator(std::string_view computeSrc);
    ~TerrainGenerator();

    TerrainGenerator(const TerrainGenerator &) = delete;
    TerrainGenerator &operator=(const TerrainGenerator &) = delete;

    std::optional<GpuJob> dispatchJob(const glm::ivec3 &chunkCoord);
    void readJobResults(const GpuJob &job, uint32_t *out_blockData);
    void releaseJobResources(const GpuJob &job);
};
