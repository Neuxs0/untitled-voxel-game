#pragma once

#include <cstdint>

// Represents an allocation of a mesh within a larger buffer pool.
struct MeshAllocation
{
    // The offset (in vertices) from the beginning of the VBO.
    uint32_t vertexOffset = 0;
    // The offset (in indices) from the beginning of the EBO.
    uint32_t indexOffset = 0;
    // The number of vertices in this mesh.
    uint32_t vertexCount = 0;
    // The number of indices in this mesh.
    uint32_t indexCount = 0;

    // Checks if the allocation is valid (i.e., has something to draw).
    bool isValid() const
    {
        return indexCount > 0 && vertexCount > 0;
    }
};
