#include "TerrainGenerator.hpp"
#include "Constants.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring> // Required for memcpy

// Constructor
TerrainGenerator::TerrainGenerator(std::string_view computeSrc)
{
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char *srcData = computeSrc.data();
    GLint srcLength = static_cast<GLint>(computeSrc.size());
    glShaderSource(computeShader, 1, &srcData, &srcLength);
    glCompileShader(computeShader);
    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        std::cerr << "Error: TerrainGenerator: Compute shader compilation failed:\n" << infoLog << std::endl;
        throw std::runtime_error("Compute shader compilation failed.");
    }

    m_computeProgramID = glCreateProgram();
    glAttachShader(m_computeProgramID, computeShader);
    glLinkProgram(m_computeProgramID);
    glGetProgramiv(m_computeProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(m_computeProgramID, 512, NULL, infoLog);
        std::cerr << "Error: TerrainGenerator: Compute program linking failed:\n" << infoLog << std::endl;
        glDeleteShader(computeShader);
        throw std::runtime_error("Compute program linking failed.");
    }
    glDeleteShader(computeShader);

    constexpr size_t bufferSize = Constants::CHUNK_VOL * sizeof(uint32_t);

    // Initialize SSBO pool for compute shaders
    m_ssboPool.resize(MAX_CONCURRENT_JOBS);
    glGenBuffers(MAX_CONCURRENT_JOBS, m_ssboPool.data());
    for (int i = 0; i < MAX_CONCURRENT_JOBS; ++i)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboPool[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        m_freeSsboQueue.push_back(m_ssboPool[i]);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Initialize PBO pool for asynchronous read-back
    m_pboPool.resize(MAX_CONCURRENT_JOBS);
    glGenBuffers(MAX_CONCURRENT_JOBS, m_pboPool.data());
    for (int i = 0; i < MAX_CONCURRENT_JOBS; ++i)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pboPool[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, nullptr, GL_STREAM_READ);
        m_freePboQueue.push_back(m_pboPool[i]);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

// Destructor
TerrainGenerator::~TerrainGenerator()
{
    glDeleteProgram(m_computeProgramID);
    glDeleteBuffers(m_ssboPool.size(), m_ssboPool.data());
    glDeleteBuffers(m_pboPool.size(), m_pboPool.data());
}

// Tries to dispatch a new job to the GPU.
std::optional<GpuJob> TerrainGenerator::dispatchJob(const glm::ivec3 &chunkCoord)
{
    if (m_freeSsboQueue.empty())
    {
        return std::nullopt;
    }

    GLuint ssbo = m_freeSsboQueue.front();
    m_freeSsboQueue.pop_front();

    glUseProgram(m_computeProgramID);
    glUniform3i(glGetUniformLocation(m_computeProgramID, "u_chunkCoord"), chunkCoord.x, chunkCoord.y, chunkCoord.z);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glDispatchCompute(Constants::CHUNK_DIM / 8, Constants::CHUNK_DIM / 8, Constants::CHUNK_DIM / 8);
    GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    return GpuJob{chunkCoord, ssbo, fence};
}

// Schedules a non-blocking copy from the finished job's SSBO to a PBO.
std::optional<PboReadJob> TerrainGenerator::scheduleRead(const GpuJob& finishedJob)
{
    if (m_freePboQueue.empty()) {
        // PBO pool is full, cannot schedule the read. Try again next frame.
        return std::nullopt;
    }
    GLuint pbo = m_freePboQueue.front();
    m_freePboQueue.pop_front();

    // Perform an asynchronous GPU-to-GPU copy.
    glBindBuffer(GL_COPY_READ_BUFFER, finishedJob.ssbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, pbo);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, Constants::CHUNK_VOL * sizeof(uint32_t));
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

    // Create a new fence that will be signaled when the copy operation completes.
    GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    return PboReadJob{finishedJob.chunkCoord, pbo, fence};
}

// Reads data from a PBO that has finished its transfer.
void TerrainGenerator::readPboData(const PboReadJob& job, uint32_t* out_blockData)
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, job.pbo);
    // Map the buffer. Since we waited for the fence, this should not stall.
    void* ptr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, Constants::CHUNK_VOL * sizeof(uint32_t), GL_MAP_READ_BIT);
    if (ptr) {
        memcpy(out_blockData, ptr, Constants::CHUNK_VOL * sizeof(uint32_t));
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    } else {
        std::cerr << "Error: glMapBufferRange failed for PBO " << job.pbo << std::endl;
        // As a fallback, a robust implementation could fill the output buffer with default data (e.g., AIR blocks).
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void TerrainGenerator::releaseGpuJob(const GpuJob& job)
{
    glDeleteSync(job.fence);
    m_freeSsboQueue.push_back(job.ssbo);
}

void TerrainGenerator::releasePboJob(const PboReadJob& job)
{
    glDeleteSync(job.fence);
    m_freePboQueue.push_back(job.pbo);
}

bool TerrainGenerator::hasAvailableJobSlots() const
{
    return !m_freeSsboQueue.empty();
}
