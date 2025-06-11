#pragma once

#include <vector>
#include <glm/glm.hpp>

class Mesh;
class Shader;
class World;

#include "Constants.hpp"
#include "Vertex.hpp"
#include "Block.hpp"

// Represents a chunk of the world.
class Chunk
{
private:
    // The world position of the chunk's origin corner.
    glm::vec3 m_position;
    // The coordinate of this chunk in the world grid.
    glm::ivec3 m_chunkCoord;
    // The blocks in this chunk.
    BlockType m_blocks[Constants::CHUNK_DIM][Constants::CHUNK_DIM][Constants::CHUNK_DIM];
    // The mesh for opaque blocks.
    Mesh *m_opaqueMesh;
    // The mesh for transparent blocks.
    Mesh *m_transparentMesh;

public:
    // Constructor.
    Chunk(glm::ivec3 chunkCoord);
    // Destructor.
    ~Chunk();

    // Returns the block at the given local coordinates.
    BlockType getBlock(int x, int y, int z) const;

    // Generates the mesh for this chunk.
    void generateMesh(const World &world);

    // Renders the opaque blocks in this chunk.
    void renderOpaque(Shader &shader);
    // Renders the transparent blocks in this chunk.
    void renderTransparent(Shader &shader);
};
