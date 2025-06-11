#pragma once

#include <vector>
#include <glm/glm.hpp>

// Forward declarations to avoid including full headers here
class Mesh;
class Shader;

#include "Constants.hpp"
#include "Vertex.hpp"
#include "Block.hpp"

class Chunk {
private:
    // Returns the block type at a local chunk coordinate
    BlockType getBlock(int x, int y, int z) const;

    // World position of the chunk's origin (corner)
    glm::vec3 m_position;

    // 3D array storing all block types in this chunk
    BlockType m_blocks[Constants::CHUNK_DIM][Constants::CHUNK_DIM][Constants::CHUNK_DIM];

    Mesh *m_opaqueMesh;
    Mesh *m_transparentMesh;

public:
    Chunk(glm::vec3 position);
    ~Chunk();

    void generateMesh();

    void renderOpaque(Shader &shader);
    void renderTransparent(Shader &shader);
};
