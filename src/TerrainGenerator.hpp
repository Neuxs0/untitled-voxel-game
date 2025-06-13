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

// A structure to track a pending asynchronous read-back via a PBO.
struct PboReadJob
{
    glm::ivec3 chunkCoord;
    GLuint pbo;
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

    // A pool of PBOs for asynchronous read-back of chunk data.
    std::vector<GLuint> m_pboPool;
    std::deque<GLuint> m_freePboQueue;

    // The maximum number of chunk generations that can be in-flight on the GPU.
    static constexpr int MAX_CONCURRENT_JOBS = 64;

public:
    TerrainGenerator(std::string_view computeSrc);
    ~TerrainGenerator();

    TerrainGenerator(const TerrainGenerator &) = delete;
    TerrainGenerator &operator=(const TerrainGenerator &) = delete;

    // Dispatches a new compute job to the GPU.
    std::optional<GpuJob> dispatchJob(const glm::ivec3 &chunkCoord);
    
    // Schedules a non-blocking copy from the finished job's SSBO to a PBO.
    std::optional<PboReadJob> scheduleRead(const GpuJob& finishedJob);

    // Reads data from a PBO that has finished its transfer.
    void readPboData(const PboReadJob& job, uint32_t* out_blockData);

    // Releases resources for a finished GPU compute job.
    void releaseGpuJob(const GpuJob& job);
    
    // Releases resources for a finished PBO read-back job.
    void releasePboJob(const PboReadJob& job);

    // Checks if there are free SSBOs to dispatch new jobs.
    bool hasAvailableJobSlots() const;
};
