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
    // Clear old mesh data
    delete m_opaqueMesh;
    m_opaqueMesh = nullptr;
    delete m_transparentMesh;
    m_transparentMesh = nullptr;

    std::vector<Vertex> opaqueVertices;
    std::vector<Vertex> transparentVertices;

    // Iterate over the 3 dimensions (x, y, z)
    for (int d = 0; d < 3; ++d)
    {
        int u = (d + 1) % 3;
        int v = (d + 2) % 3;

        // Iterate over the 2 directions (positive and negative)
        for (int dir = -1; dir <= 1; dir += 2)
        {
            glm::vec3 normal = {0, 0, 0};
            normal[d] = static_cast<float>(dir);

            // Sweep a plane through the chunk for the current axis and direction
            for (int i = 0; i < Constants::CHUNK_DIM; ++i)
            {
                BlockType mask[Constants::CHUNK_DIM][Constants::CHUNK_DIM] = {{BlockType::AIR}};

                // 1. Create a 2D mask of visible faces on the current slice
                for (int j = 0; j < Constants::CHUNK_DIM; ++j)
                {
                    for (int k = 0; k < Constants::CHUNK_DIM; ++k)
                    {
                        glm::ivec3 currentPos(0), neighborPos(0);
                        currentPos[d] = i; currentPos[u] = j; currentPos[v] = k;
                        neighborPos = currentPos; neighborPos[d] += dir;

                        BlockType currentType = getBlock(currentPos.x, currentPos.y, currentPos.z);
                        BlockType neighborType = getBlock(neighborPos.x, neighborPos.y, neighborPos.z);

                        BlockData currentData = Block::get(currentType);
                        BlockData neighborData = Block::get(neighborType);

                        bool currentIsVisible = currentType != BlockType::AIR;
                        bool neighborOccludes = neighborType != BlockType::AIR && neighborData.isOpaque;

                        // A face is visible if the current block is not air, and it's not occluded by an opaque neighbor.
                        // We also handle the case where an opaque block's face is visible through a transparent one.
                        if (currentIsVisible && (neighborType == BlockType::AIR || (!neighborData.isOpaque && currentData.isOpaque)))
                        {
                            mask[j][k] = currentType;
                        }
                    }
                }

                // 2. Process the mask to generate greedy quads
                for (int j = 0; j < Constants::CHUNK_DIM; ++j)
                {
                    for (int k = 0; k < Constants::CHUNK_DIM;)
                    {
                        BlockType currentType = mask[j][k];
                        if (currentType == BlockType::AIR)
                        {
                            k++;
                            continue;
                        }

                        // Find width (w)
                        int w = 1;
                        while (k + w < Constants::CHUNK_DIM && mask[j][k + w] == currentType)
                            w++;

                        // Find height (h)
                        int h = 1;
                        bool done = false;
                        while (j + h < Constants::CHUNK_DIM && !done)
                        {
                            for (int m = 0; m < w; ++m)
                            {
                                if (mask[j + h][k + m] != currentType)
                                {
                                    done = true;
                                    break;
                                }
                            }
                            if (!done)
                                h++;
                        }

                        // We have a quad of type 'currentType' at (j, k) with size (h, w)
                        BlockData blockData = Block::get(currentType);
                        std::vector<Vertex>& targetVertices = blockData.isOpaque ? opaqueVertices : transparentVertices;
                        glm::vec4 color = blockData.color;

                        // Calculate the four corner vertices of the quad
                        glm::vec3 du = {0,0,0}, dv = {0,0,0};
                        du[u] = 1.0f; dv[v] = 1.0f;

                        glm::vec3 quad_start_pos(0.0f);
                        quad_start_pos[d] = static_cast<float>(i + (dir > 0 ? 1 : 0));
                        quad_start_pos[u] = static_cast<float>(j);
                        quad_start_pos[v] = static_cast<float>(k);
                        
                        glm::vec3 p1 = quad_start_pos;
                        glm::vec3 p2 = quad_start_pos + (static_cast<float>(h) * du);
                        glm::vec3 p3 = quad_start_pos + (static_cast<float>(h) * du) + (static_cast<float>(w) * dv);
                        glm::vec3 p4 = quad_start_pos + (static_cast<float>(w) * dv);

                        // Transform from grid corner coordinates to centered mesh coordinates
                        const glm::vec3 centerOffset(Constants::CHUNK_DIM / 2.0f);
                        p1 = (p1 - centerOffset) * Constants::BLOCK_WIDTH;
                        p2 = (p2 - centerOffset) * Constants::BLOCK_WIDTH;
                        p3 = (p3 - centerOffset) * Constants::BLOCK_WIDTH;
                        p4 = (p4 - centerOffset) * Constants::BLOCK_WIDTH;
                        
                        // Add the 6 vertices for the two triangles with correct winding order
                        if (dir > 0) {
                            targetVertices.insert(targetVertices.end(), {{p1, color, normal}, {p2, color, normal}, {p3, color, normal}, {p3, color, normal}, {p4, color, normal}, {p1, color, normal}});
                        } else {
                            targetVertices.insert(targetVertices.end(), {{p1, color, normal}, {p4, color, normal}, {p3, color, normal}, {p3, color, normal}, {p2, color, normal}, {p1, color, normal}});
                        }

                        // Clear the mask for the area we just covered
                        for (int l = 0; l < h; ++l)
                            for (int m = 0; m < w; ++m)
                                mask[j + l][k + m] = BlockType::AIR;
                        
                        k += w;
                    }
                }
            }
        }
    }

    // Create new meshes if their vertex lists are not empty
    if (!opaqueVertices.empty())
        m_opaqueMesh = new Mesh(opaqueVertices);

    if (!transparentVertices.empty())
        m_transparentMesh = new Mesh(transparentVertices);
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
