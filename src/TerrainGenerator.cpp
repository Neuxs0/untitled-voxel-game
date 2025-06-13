#include "TerrainGenerator.hpp"
#include "Constants.hpp"
#include <iostream>
#include <stdexcept>

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
    if (!success) { /* error handling */ }

    m_computeProgramID = glCreateProgram();
    glAttachShader(m_computeProgramID, computeShader);
    glLinkProgram(m_computeProgramID);
    glGetProgramiv(m_computeProgramID, GL_LINK_STATUS, &success);
    if (!success) { /* error handling */ }
    glDeleteShader(computeShader);

    m_ssboPool.resize(MAX_CONCURRENT_JOBS);
    glGenBuffers(MAX_CONCURRENT_JOBS, m_ssboPool.data());
    constexpr size_t bufferSize = Constants::CHUNK_VOL * sizeof(uint32_t);
    for (int i = 0; i < MAX_CONCURRENT_JOBS; ++i)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboPool[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        m_freeSsboQueue.push_back(m_ssboPool[i]);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Destructor
TerrainGenerator::~TerrainGenerator()
{
    glDeleteProgram(m_computeProgramID);
    glDeleteBuffers(m_ssboPool.size(), m_ssboPool.data());
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

void TerrainGenerator::readJobResults(const GpuJob &job, uint32_t *out_blockData)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, job.ssbo);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, Constants::CHUNK_VOL * sizeof(uint32_t), out_blockData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void TerrainGenerator::releaseJobResources(const GpuJob& job)
{
    glDeleteSync(job.fence);
    m_freeSsboQueue.push_back(job.ssbo);
}
