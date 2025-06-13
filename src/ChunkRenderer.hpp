#pragma once

#include <GL/glew.h>
#include <vector>
#include <list>

#include "MeshAllocation.hpp"

// Forward-declaration of Vertex struct.
// The header file only needs to know that the type 'Vertex' exists.
// The full definition is only required in the .cpp file.
struct Vertex;

class ChunkRenderer
{
private:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;

    // Represents a contiguous block of free space in the VBO and EBO.
    struct BufferBlock
    {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t vertexCapacity;
        uint32_t indexCapacity;
    };

    const uint32_t m_vertexCapacity;
    const uint32_t m_indexCapacity;

    // A list of free blocks, kept sorted by offset to allow for merging.
    std::list<BufferBlock> m_freeList;

    // Private helper methods
    void initializeBuffers();
    void mergeFreeBlocks(std::list<BufferBlock>::iterator it);

public:
    ChunkRenderer(uint32_t vertexCapacity, uint32_t indexCapacity);
    ~ChunkRenderer();

    // Prevent copying and moving to avoid issues with OpenGL resource ownership.
    ChunkRenderer(const ChunkRenderer &) = delete;
    ChunkRenderer &operator=(const ChunkRenderer &) = delete;
    ChunkRenderer(ChunkRenderer &&) = delete;
    ChunkRenderer &operator=(ChunkRenderer &&) = delete;

    /**
     * @brief Allocates space in the GPU buffers for a mesh.
     * @param vertices The vertex data for the mesh.
     * @param indices The index data for the mesh.
     * @return A MeshAllocation struct representing the location and size of the allocated mesh.
     */
    MeshAllocation allocateMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
    
    /**
     * @brief Frees a previously allocated mesh, making its space available for reuse.
     * @param allocation The MeshAllocation struct to free.
     */
    void freeMesh(const MeshAllocation &allocation);

    /**
     * @brief Binds the renderer's Vertex Array Object for drawing.
     */
    void bind() const;

    /**
     * @brief Unbinds the renderer's Vertex Array Object.
     */
    void unbind() const;
    
    /**
     * @brief Issues a draw call for a specific mesh allocation.
     * @param allocation The mesh to draw.
     */
    static void draw(const MeshAllocation &allocation);
};
