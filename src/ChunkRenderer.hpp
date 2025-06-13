#pragma once

#include <GL/glew.h>
#include <vector>
#include <list>
#include <optional>

#include "MeshAllocation.hpp"

// Forward-declaration
struct Vertex;

class ChunkRenderer
{
private:
    // Represents a contiguous block of free space in a VBO and EBO.
    struct BufferBlock
    {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t vertexCapacity;
        uint32_t indexCapacity;
    };

    // A set of GPU buffers (VAO, VBO, EBO) and its associated memory manager.
    struct BufferPool
    {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        uint32_t vertexCapacity;
        uint32_t indexCapacity;
        // A list of free blocks, kept sorted by offset to allow for merging.
        std::list<BufferBlock> freeList{};
    };

    std::vector<BufferPool> m_pools;
    const uint32_t m_poolVertexCapacity;
    const uint32_t m_poolIndexCapacity;
    
    // Tracks the last bound VAO to avoid redundant binds.
    mutable GLuint m_lastBoundVao = 0;

    // Private helper methods
    void createNewPool();
    void initializePool(BufferPool& pool);
    std::optional<MeshAllocation> tryAllocateInPool(uint32_t poolIndex, const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices);
    void mergeFreeBlocks(BufferPool& pool, std::list<BufferBlock>::iterator it);

public:
    ChunkRenderer(uint32_t poolVertexCapacity, uint32_t poolIndexCapacity);
    ~ChunkRenderer();

    // Prevent copying and moving to avoid issues with OpenGL resource ownership.
    ChunkRenderer(const ChunkRenderer &) = delete;
    ChunkRenderer &operator=(const ChunkRenderer &) = delete;
    ChunkRenderer(ChunkRenderer &&) = delete;
    ChunkRenderer &operator=(ChunkRenderer &&) = delete;

    /**
     * @brief Allocates space in the GPU buffers for a mesh. May create a new buffer pool if needed.
     * @param vertices The vertex data for the mesh.
     * @param indices The index data for the mesh.
     * @return A MeshAllocation struct representing the location and size of the allocated mesh.
     */
    MeshAllocation allocateMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned short> &indices);
    
    /**
     * @brief Frees a previously allocated mesh, making its space available for reuse.
     * @param allocation The MeshAllocation struct to free.
     */
    void freeMesh(const MeshAllocation &allocation);
    
    /**
     * @brief Issues a draw call for a specific mesh allocation, binding the correct VAO if necessary.
     * @param allocation The mesh to draw.
     */
    void draw(const MeshAllocation &allocation) const;
};
