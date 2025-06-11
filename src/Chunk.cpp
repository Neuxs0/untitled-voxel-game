#include "Chunk.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include <vector>

// Constructor that populates the chunk with a simple terrain
Chunk::Chunk(glm::vec3 position) : m_position(position), m_mesh(nullptr)
{
    for (int x = 0; x < CHUNK_WIDTH; ++x)
    {
        for (int z = 0; z < CHUNK_DEPTH; ++z)
        {
            for (int y = 0; y < CHUNK_HEIGHT; ++y)
            {
                // Generate a simple layered world
                if (y < CHUNK_HEIGHT / 2)
                    m_blocks[x][y][z] = BlockType::STONE;
                else if (y < CHUNK_HEIGHT - 2)
                    m_blocks[x][y][z] = BlockType::DIRT;
                else if (y < CHUNK_HEIGHT - 1)
                    m_blocks[x][y][z] = BlockType::GRASS;
                else if (y == CHUNK_HEIGHT - 1)
                    // Create a layer of water on top
                    m_blocks[x][y][z] = BlockType::WATER;
                else
                    m_blocks[x][y][z] = BlockType::AIR;
            }
        }
    }
}

Chunk::~Chunk() { delete m_mesh; }

BlockType Chunk::getBlock(int x, int y, int z) const
{
    // Check coordinate bounds. If outside the chunk, treat it as AIR.
    if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH)
        return BlockType::AIR;
    
    return m_blocks[x][y][z];
}

void Chunk::generateMesh()
{
    std::vector<Vertex> vertices;

    // Iterate through every block in the chunk to build the mesh
    for (int x = 0; x < CHUNK_WIDTH; ++x)
    {
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_DEPTH; ++z)
            {
                BlockType currentBlockType = getBlock(x, y, z);

                // Skip air blocks, as they are not visible
                if (currentBlockType == BlockType::AIR)
                    continue;

                glm::vec3 blockPos(x, y, z);

                // A face is visible if it's adjacent to a non-opaque block (e.g., air or water)
                // Check neighbor in +Z direction (Front face)
                if (!Block::get(getBlock(x, y, z + 1)).isOpaque)
                    addFace(vertices, blockPos, currentBlockType, {0, 0, 1});
                // Check neighbor in -Z direction (Back face)
                if (!Block::get(getBlock(x, y, z - 1)).isOpaque)
                    addFace(vertices, blockPos, currentBlockType, {0, 0, -1});
                // Check neighbor in +X direction (Right face)
                if (!Block::get(getBlock(x + 1, y, z)).isOpaque)
                    addFace(vertices, blockPos, currentBlockType, {1, 0, 0});
                // Check neighbor in -X direction (Left face)
                if (!Block::get(getBlock(x - 1, y, z)).isOpaque)
                    addFace(vertices, blockPos, currentBlockType, {-1, 0, 0});
                // Check neighbor in +Y direction (Top face)
                if (!Block::get(getBlock(x, y + 1, z)).isOpaque)
                    addFace(vertices, blockPos, currentBlockType, {0, 1, 0});
                // Check neighbor in -Y direction (Bottom face)
                if (!Block::get(getBlock(x, y - 1, z)).isOpaque)
                    addFace(vertices, blockPos, currentBlockType, {0, -1, 0});
            }
        }
    }

    // Delete the old mesh data before creating a new one
    delete m_mesh;

    // Create a new mesh if there are any vertices to draw
    if (!vertices.empty())
    {
        // NOTE: This requires a new constructor in Mesh.hpp
        m_mesh = new Mesh(vertices);
    }
    else
    {
        m_mesh = nullptr;
    }
}

void Chunk::render(Shader &shader)
{
    if (m_mesh)
    {
        // Set the ModelMatrix uniform for this specific chunk's position
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), m_position);
        shader.setMat4("ModelMatrix", ModelMatrix);

        m_mesh->render();
    }
}

// Generates the 6 vertices for a single quad face
void Chunk::addFace(std::vector<Vertex> &vertices, const glm::vec3 &blockPos, BlockType type, const glm::vec3 &normal)
{
    glm::vec4 color = Block::get(type).color;
    float s = 0.5f; // Block half-size

    // Define the 4 corners of a face based on its normal vector
    glm::vec3 p1, p2, p3, p4;

    if (normal == glm::vec3(0, 0, 1))
    { // Front
        p1 = blockPos + glm::vec3(-s, -s, s);
        p2 = blockPos + glm::vec3(s, -s, s);
        p3 = blockPos + glm::vec3(s, s, s);
        p4 = blockPos + glm::vec3(-s, s, s);
    }
    else if (normal == glm::vec3(0, 0, -1))
    { // Back
        p1 = blockPos + glm::vec3(s, -s, -s);
        p2 = blockPos + glm::vec3(-s, -s, -s);
        p3 = blockPos + glm::vec3(-s, s, -s);
        p4 = blockPos + glm::vec3(s, s, -s);
    }
    else if (normal == glm::vec3(1, 0, 0))
    { // Right
        p1 = blockPos + glm::vec3(s, -s, s);
        p2 = blockPos + glm::vec3(s, -s, -s);
        p3 = blockPos + glm::vec3(s, s, -s);
        p4 = blockPos + glm::vec3(s, s, s);
    }
    else if (normal == glm::vec3(-1, 0, 0))
    { // Left
        p1 = blockPos + glm::vec3(-s, -s, -s);
        p2 = blockPos + glm::vec3(-s, -s, s);
        p3 = blockPos + glm::vec3(-s, s, s);
        p4 = blockPos + glm::vec3(-s, s, -s);
    }
    else if (normal == glm::vec3(0, 1, 0))
    { // Top
        p1 = blockPos + glm::vec3(-s, s, s);
        p2 = blockPos + glm::vec3(s, s, s);
        p3 = blockPos + glm::vec3(s, s, -s);
        p4 = blockPos + glm::vec3(-s, s, -s);
    }
    else
    { // Bottom
        p1 = blockPos + glm::vec3(-s, -s, -s);
        p2 = blockPos + glm::vec3(s, -s, -s);
        p3 = blockPos + glm::vec3(s, -s, s);
        p4 = blockPos + glm::vec3(-s, -s, s);
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
