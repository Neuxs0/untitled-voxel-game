#include "ChunkRenderer.hpp"
#include <iostream>
#include <algorithm>

// This include is necessary for the definition of the Vertex struct.
#include "Vertex.hpp"

ChunkRenderer::ChunkRenderer(uint32_t vertexCapacity, uint32_t indexCapacity)
    : m_vao(0), m_vbo(0), m_ebo(0), m_vertexCapacity(vertexCapacity), m_indexCapacity(indexCapacity)
{
    initializeBuffers();
}

ChunkRenderer::~ChunkRenderer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void ChunkRenderer::initializeBuffers()
{
    glCreateVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexCapacity * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCapacity * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Initialize the free list with a single block covering the entire buffer.
    m_freeList.push_back({0, 0, m_vertexCapacity, m_indexCapacity});
}

MeshAllocation ChunkRenderer::allocateMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
{
    const uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
    const uint32_t indexCount = static_cast<uint32_t>(indices.size());

    // Find the first free block that is large enough
    for (auto it = m_freeList.begin(); it != m_freeList.end(); ++it)
    {
        if (it->vertexCapacity >= vertexCount && it->indexCapacity >= indexCount)
        {
            BufferBlock block = *it;
            m_freeList.erase(it);

            MeshAllocation allocation = {
                block.vertexOffset,
                block.indexOffset,
                vertexCount,
                indexCount};

            uint32_t remainingVertices = block.vertexCapacity - vertexCount;
            uint32_t remainingIndices = block.indexCapacity - indexCount;

            // If there's leftover space, add it back to the free list as a new, smaller block.
            if (remainingVertices > 0 || remainingIndices > 0)
            {
                BufferBlock newFreeBlock = {
                    block.vertexOffset + vertexCount,
                    block.indexOffset + indexCount,
                    remainingVertices,
                    remainingIndices};
                // Insert the new free block in sorted order to help prevent fragmentation.
                auto insert_pos = std::lower_bound(m_freeList.begin(), m_freeList.end(), newFreeBlock.vertexOffset,
                                                   [](const BufferBlock &b, uint32_t offset)
                                                   { return b.vertexOffset < offset; });
                m_freeList.insert(insert_pos, newFreeBlock);
            }

            // Upload the vertex and index data to the GPU.
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, allocation.vertexOffset * sizeof(Vertex), vertexCount * sizeof(Vertex), vertices.data());

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, allocation.indexOffset * sizeof(unsigned int), indexCount * sizeof(unsigned int), indices.data());

            return allocation;
        }
    }

    std::cerr << "ChunkRenderer Error: Failed to allocate mesh. Buffer pool may be full." << std::endl;
    return {}; // Return an invalid allocation
}

void ChunkRenderer::freeMesh(const MeshAllocation &allocation)
{
    if (!allocation.isValid())
        return;

    BufferBlock freedBlock = {
        allocation.vertexOffset,
        allocation.indexOffset,
        allocation.vertexCount,
        allocation.indexCount};

    // Find the correct sorted position to insert the newly freed block.
    auto it = std::lower_bound(m_freeList.begin(), m_freeList.end(), freedBlock.vertexOffset,
                               [](const BufferBlock &b, uint32_t offset)
                               { return b.vertexOffset < offset; });

    auto inserted_it = m_freeList.insert(it, freedBlock);

    // Try to merge the newly freed block with its neighbors.
    mergeFreeBlocks(inserted_it);
}

void ChunkRenderer::mergeFreeBlocks(std::list<BufferBlock>::iterator it)
{
    // Try to merge with the previous block
    if (it != m_freeList.begin())
    {
        auto prev = std::prev(it);
        // Check if the blocks are physically adjacent in the buffer
        if (prev->vertexOffset + prev->vertexCapacity == it->vertexOffset &&
            prev->indexOffset + prev->indexCapacity == it->indexOffset)
        {
            // Merge them
            prev->vertexCapacity += it->vertexCapacity;
            prev->indexCapacity += it->indexCapacity;
            it = m_freeList.erase(it);
            it = prev; // Update iterator to the newly merged block
        }
    }

    // Try to merge with the next block
    auto next = std::next(it);
    if (next != m_freeList.end())
    {
        // Check if the blocks are physically adjacent
        if (it->vertexOffset + it->vertexCapacity == next->vertexOffset &&
            it->indexOffset + it->indexCapacity == next->indexOffset)
        {
            // Merge them
            it->vertexCapacity += next->vertexCapacity;
            it->indexCapacity += next->indexCapacity;
            m_freeList.erase(next);
        }
    }
}

void ChunkRenderer::bind() const
{
    glBindVertexArray(m_vao);
}

void ChunkRenderer::unbind() const
{
    glBindVertexArray(0);
}

void ChunkRenderer::draw(const MeshAllocation &allocation)
{
    glDrawElementsBaseVertex(
        GL_TRIANGLES,
        allocation.indexCount,
        GL_UNSIGNED_INT,
        (void *)(sizeof(unsigned int) * allocation.indexOffset),
        allocation.vertexOffset);
}
