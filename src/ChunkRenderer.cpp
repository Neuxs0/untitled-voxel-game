#include "ChunkRenderer.hpp"
#include <iostream>
#include <algorithm>
#include <optional>

// Includes the Vertex struct definition, required for buffer layouts and data uploads.
#include "Vertex.hpp"

// Constructor
ChunkRenderer::ChunkRenderer(uint32_t poolVertexCapacity, uint32_t poolIndexCapacity)
    : m_poolVertexCapacity(poolVertexCapacity), m_poolIndexCapacity(poolIndexCapacity)
{
    // Start with one pool.
    createNewPool();
}

// Destructor
ChunkRenderer::~ChunkRenderer()
{
    for (auto& pool : m_pools) {
        glDeleteVertexArrays(1, &pool.vao);
        glDeleteBuffers(1, &pool.vbo);
        glDeleteBuffers(1, &pool.ebo);
    }
}

// Creates and initializes a new, empty BufferPool.
void ChunkRenderer::createNewPool()
{
    m_pools.emplace_back(BufferPool{
        .vertexCapacity = m_poolVertexCapacity,
        .indexCapacity = m_poolIndexCapacity
    });
    initializePool(m_pools.back());
    
    m_lastBoundVao = 0;

    std::cout << "ChunkRenderer: No space found, creating new buffer pool (ID: " << m_pools.size() - 1 << ")." << std::endl;
}


// Sets up the OpenGL buffers for a given pool.
void ChunkRenderer::initializePool(BufferPool& pool)
{
    glCreateVertexArrays(1, &pool.vao);
    glBindVertexArray(pool.vao);

    glGenBuffers(1, &pool.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pool.vbo);
    glBufferData(GL_ARRAY_BUFFER, pool.vertexCapacity * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &pool.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pool.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pool.indexCapacity * sizeof(unsigned short), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute (vec3) - Location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Color attribute (4x uint8_t, normalized) - Location 1
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    
    // Normal attribute (3x int8_t, normalized) - Location 2
    glVertexAttribPointer(2, 3, GL_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    // Atlas Offset attribute (vec2) - Location 3
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, atlasOffset));
    glEnableVertexAttribArray(3);

    // Surface Coordinates attribute (vec2) - Location 4
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, surfaceCoords));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    // Initialize the free list with a single block covering the entire buffer.
    pool.freeList.push_back({0, 0, pool.vertexCapacity, pool.indexCapacity});
}

// Tries to allocate a mesh within a specific pool.
std::optional<MeshAllocation> ChunkRenderer::tryAllocateInPool(uint32_t poolIndex, const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices)
{
    BufferPool& pool = m_pools[poolIndex];
    const uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
    const uint32_t indexCount = static_cast<uint32_t>(indices.size());

    // Find the first free block that is large enough
    for (auto it = pool.freeList.begin(); it != pool.freeList.end(); ++it)
    {
        if (it->vertexCapacity >= vertexCount && it->indexCapacity >= indexCount)
        {
            BufferBlock block = *it;
            pool.freeList.erase(it);

            MeshAllocation allocation = {
                poolIndex,
                block.vertexOffset,
                block.indexOffset,
                vertexCount,
                indexCount};

            uint32_t remainingVertices = block.vertexCapacity - vertexCount;
            uint32_t remainingIndices = block.indexCapacity - indexCount;

            if (remainingVertices > 0 || remainingIndices > 0)
            {
                BufferBlock newFreeBlock = {
                    block.vertexOffset + vertexCount,
                    block.indexOffset + indexCount,
                    remainingVertices,
                    remainingIndices};
                auto insert_pos = std::lower_bound(pool.freeList.begin(), pool.freeList.end(), newFreeBlock.vertexOffset,
                                                   [](const BufferBlock &b, uint32_t offset)
                                                   { return b.vertexOffset < offset; });
                pool.freeList.insert(insert_pos, newFreeBlock);
            }

            // Use Direct State Access (DSA) to upload data without binding
            glNamedBufferSubData(pool.vbo, allocation.vertexOffset * sizeof(Vertex), vertexCount * sizeof(Vertex), vertices.data());
            glNamedBufferSubData(pool.ebo, allocation.indexOffset * sizeof(unsigned short), indexCount * sizeof(unsigned short), indices.data());

            return allocation;
        }
    }

    return std::nullopt;
}

MeshAllocation ChunkRenderer::allocateMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned short> &indices)
{
    // Try to allocate in existing pools first.
    for (size_t i = 0; i < m_pools.size(); ++i) {
        if (auto allocation = tryAllocateInPool(i, vertices, indices)) {
            return *allocation;
        }
    }

    // If no space was found, create a new pool.
    createNewPool();

    if (auto allocation = tryAllocateInPool(m_pools.size() - 1, vertices, indices)) {
        return *allocation;
    }

    std::cerr << "ChunkRenderer CRITICAL ERROR: Failed to allocate mesh even after creating a new pool." << std::endl;
    return {};
}


void ChunkRenderer::freeMesh(const MeshAllocation &allocation)
{
    if (!allocation.isValid() || allocation.poolIndex >= m_pools.size())
        return;

    BufferPool& pool = m_pools[allocation.poolIndex];

    BufferBlock freedBlock = {
        allocation.vertexOffset,
        allocation.indexOffset,
        allocation.vertexCount,
        allocation.indexCount};

    auto it = std::lower_bound(pool.freeList.begin(), pool.freeList.end(), freedBlock.vertexOffset,
                               [](const BufferBlock &b, uint32_t offset)
                               { return b.vertexOffset < offset; });

    auto inserted_it = pool.freeList.insert(it, freedBlock);
    mergeFreeBlocks(pool, inserted_it);
}

void ChunkRenderer::mergeFreeBlocks(BufferPool& pool, std::list<BufferBlock>::iterator it)
{
    if (it != pool.freeList.begin())
    {
        auto prev = std::prev(it);
        if (prev->vertexOffset + prev->vertexCapacity == it->vertexOffset &&
            prev->indexOffset + prev->indexCapacity == it->indexOffset)
        {
            prev->vertexCapacity += it->vertexCapacity;
            prev->indexCapacity += it->indexCapacity;
            it = pool.freeList.erase(it);
            it = prev;
        }
    }
    auto next = std::next(it);
    if (next != pool.freeList.end())
    {
        if (it->vertexOffset + it->vertexCapacity == next->vertexOffset &&
            it->indexOffset + it->indexCapacity == next->indexOffset)
        {
            it->vertexCapacity += next->vertexCapacity;
            it->indexCapacity += next->indexCapacity;
            pool.freeList.erase(next);
        }
    }
}

void ChunkRenderer::draw(const MeshAllocation &allocation) const
{
    if (allocation.poolIndex >= m_pools.size()) return;
    const BufferPool& pool = m_pools[allocation.poolIndex];

    if (m_lastBoundVao != pool.vao) {
        glBindVertexArray(pool.vao);
        m_lastBoundVao = pool.vao;
    }
    
    glDrawElementsBaseVertex(
        GL_TRIANGLES,
        allocation.indexCount,
        GL_UNSIGNED_SHORT, // Use 16-bit indices
        (void *)(sizeof(unsigned short) * allocation.indexOffset),
        allocation.vertexOffset);
}
