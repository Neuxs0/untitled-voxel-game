#include "Constants.hpp"
#include "Chunk.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include <vector>

// Populates the chunk with a simple terrain
Chunk::Chunk(glm::vec3 position) : m_position(position), m_opaqueMesh(nullptr), m_transparentMesh(nullptr)
{
    for (int x = 0; x < Constants::CHUNK_DIM; ++x)
    {
        for (int z = 0; z < Constants::CHUNK_DIM; ++z)
        {
            for (int y = 0; y < Constants::CHUNK_DIM; ++y)
            {
                // Generate a simple layered world
                if (y < Constants::CHUNK_DIM / 2)
                    m_blocks[x][y][z] = BlockType::STONE;
                else if (y < Constants::CHUNK_DIM - 2)
                    m_blocks[x][y][z] = BlockType::DIRT;
                else if (y < Constants::CHUNK_DIM - 1)
                    m_blocks[x][y][z] = BlockType::GRASS;
                else if (y == Constants::CHUNK_DIM - 1)
                    // Create a layer of water on top
                    m_blocks[x][y][z] = BlockType::WATER;
                else
                    m_blocks[x][y][z] = BlockType::AIR;
            }
        }
    }
}

Chunk::~Chunk()
{
    delete m_opaqueMesh;
    delete m_transparentMesh;
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    // Check coordinate bounds. If outside the chunk, treat it as AIR.
    if (x < 0 || x >= Constants::CHUNK_DIM || y < 0 || y >= Constants::CHUNK_DIM || z < 0 || z >= Constants::CHUNK_DIM)
        return BlockType::AIR;
    
    return m_blocks[x][y][z];
}

void Chunk::generateMesh()
{
    std::vector<Vertex> opaqueVertices;
    std::vector<Vertex> transparentVertices;

    // Iterate through every block in the chunk
    for (int x = 0; x < Constants::CHUNK_DIM; ++x)
    {
        for (int y = 0; y < Constants::CHUNK_DIM; ++y)
        {
            for (int z = 0; z < Constants::CHUNK_DIM; ++z)
            {
                BlockType currentType = getBlock(x, y, z);
                if (currentType == BlockType::AIR)
                    continue;

                BlockData currentData = Block::get(currentType);
                glm::vec3 blockPos(x, y, z);

                // Decide which vertex list to add to for this block
                std::vector<Vertex> &targetVertices = currentData.isOpaque ? opaqueVertices : transparentVertices;

                // An array of neighbor offsets
                const glm::ivec3 neighbors[] = {
                    {0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};

                for (const auto &offset : neighbors)
                {
                    BlockType neighborType = getBlock(x + offset.x, y + offset.y, z + offset.z);
                    BlockData neighborData = Block::get(neighborType);

                    if (neighborType == BlockType::AIR || (currentData.isOpaque && !neighborData.isOpaque))
                        addFace(targetVertices, blockPos, currentType, glm::vec3(offset));
                }
            }
        }
    }

    // Delete old mesh data
    delete m_opaqueMesh;
    delete m_transparentMesh;

    // Create new meshes if their vertex lists are not empty
    if (!opaqueVertices.empty())
        m_opaqueMesh = new Mesh(opaqueVertices);
    else
        m_opaqueMesh = nullptr;

    if (!transparentVertices.empty())
        m_transparentMesh = new Mesh(transparentVertices);
    else
        m_transparentMesh = nullptr;
}

void Chunk::renderOpaque(Shader &shader)
{
    if (m_opaqueMesh)
    {
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), m_position);
        shader.setMat4("ModelMatrix", ModelMatrix);
        m_opaqueMesh->render();
    }
}

void Chunk::renderTransparent(Shader &shader)
{
    if (m_transparentMesh)
    {
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), m_position);
        shader.setMat4("ModelMatrix", ModelMatrix);
        m_transparentMesh->render();
    }
}

// Generates the 6 vertices for a single quad face
void Chunk::addFace(std::vector<Vertex> &vertices, const glm::vec3 &blockPos, BlockType type, const glm::vec3 &normal)
{
    glm::vec4 color = Block::get(type).color;
    float s = Constants::BLOCK_WIDTH / 2.0f;

    const glm::vec3 centerOffset(Constants::CHUNK_DIM / 2.0f - 0.5f);
    const glm::vec3 localOffset = (blockPos - centerOffset) * Constants::BLOCK_WIDTH;

    // Define the 4 corners of a face based on its normal vector
    glm::vec3 p1, p2, p3, p4;

    if (normal == glm::vec3(0, 0, 1))
    { // Front
        p1 = localOffset + glm::vec3(-s, -s, s);
        p2 = localOffset + glm::vec3(s, -s, s);
        p3 = localOffset + glm::vec3(s, s, s);
        p4 = localOffset + glm::vec3(-s, s, s);
    }
    else if (normal == glm::vec3(0, 0, -1))
    { // Back
        p1 = localOffset + glm::vec3(s, -s, -s);
        p2 = localOffset + glm::vec3(-s, -s, -s);
        p3 = localOffset + glm::vec3(-s, s, -s);
        p4 = localOffset + glm::vec3(s, s, -s);
    }
    else if (normal == glm::vec3(1, 0, 0))
    { // Right
        p1 = localOffset + glm::vec3(s, -s, s);
        p2 = localOffset + glm::vec3(s, -s, -s);
        p3 = localOffset + glm::vec3(s, s, -s);
        p4 = localOffset + glm::vec3(s, s, s);
    }
    else if (normal == glm::vec3(-1, 0, 0))
    { // Left
        p1 = localOffset + glm::vec3(-s, -s, -s);
        p2 = localOffset + glm::vec3(-s, -s, s);
        p3 = localOffset + glm::vec3(-s, s, s);
        p4 = localOffset + glm::vec3(-s, s, -s);
    }
    else if (normal == glm::vec3(0, 1, 0))
    { // Top
        p1 = localOffset + glm::vec3(-s, s, s);
        p2 = localOffset + glm::vec3(s, s, s);
        p3 = localOffset + glm::vec3(s, s, -s);
        p4 = localOffset + glm::vec3(-s, s, -s);
    }
    else
    { // Bottom
        p1 = localOffset + glm::vec3(-s, -s, -s);
        p2 = localOffset + glm::vec3(s, -s, -s);
        p3 = localOffset + glm::vec3(s, -s, s);
        p4 = localOffset + glm::vec3(-s, -s, s);
    }

    // Add two triangles to form the quad face
    // The order is important for back-face culling
    vertices.push_back({p1, color, normal});
    vertices.push_back({p2, color, normal});
    vertices.push_back({p3, color, normal});

    vertices.push_back({p3, color, normal});
    vertices.push_back({p4, color, normal});
    vertices.push_back({p1, color, normal});
}
