#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "Constants.hpp"
#include "Vertex.hpp"
#include "Block.hpp"
#include "MeshAllocation.hpp"

class World; // Forward-declaration

// The result from a CPU meshing worker thread.
struct MeshResult {
    glm::ivec3 chunkCoord;
    std::vector<Vertex> opaqueVertices;
    std::vector<unsigned short> opaqueIndices;
    std::vector<Vertex> transparentVertices;
    std::vector<unsigned short> transparentIndices;
};

// Represents an Axis-Aligned Bounding Box.
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

// Represents a chunk of the world.
class Chunk {
private:
    // The world-space coordinate of the chunk's minimum corner (e.g., where x=0, y=0, z=0 is).
    glm::vec3 m_position;
    // The world-space coordinate of the chunk's center. Used for distance sorting.
    glm::vec3 m_centerPosition;
    glm::ivec3 m_chunkCoord;
    BlockType m_blocks[Constants::CHUNK_DIM][Constants::CHUNK_DIM][Constants::CHUNK_DIM];
    MeshAllocation m_opaqueMeshAllocation;
    MeshAllocation m_transparentMeshAllocation;
    AABB m_aabb;
    AABB m_expandedAabb;

    void calculateAABB();


public:
    Chunk(glm::ivec3 chunkCoord, const uint32_t *gpuBlockData);
    ~Chunk();

    BlockType getBlock(int x, int y, int z) const;

    /**
     * @brief Generates the chunk's mesh using a greedy meshing algorithm.
     * @details This method iterates through the chunk's blocks and adjacent chunks
     *          to build an optimized mesh, merging adjacent faces of the same block type
     *          into larger quads. It separates opaque and transparent geometry.
     * @param world A const reference to the world, used to check neighbor blocks.
     * @return A MeshResult struct containing the vertex and index data for the mesh.
     */
    MeshResult generateMeshStandalone(const World &world) const;

    void setOpaqueMeshAllocation(MeshAllocation allocation);
    void setTransparentMeshAllocation(MeshAllocation allocation);

    // Getters
    const glm::vec3 &getPosition() const;
    const glm::vec3 &getCenterPosition() const;
    const AABB &getExpandedAABB() const;
    const MeshAllocation &getOpaqueMeshAllocation() const;
    const MeshAllocation &getTransparentMeshAllocation() const;
};
