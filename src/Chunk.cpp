#include "Constants.hpp"
#include "Chunk.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "World.hpp"
#include <vector>

// Chunk constructor.
Chunk::Chunk(glm::ivec3 chunkCoord)
    : m_chunkCoord(chunkCoord), m_opaqueMesh(nullptr), m_transparentMesh(nullptr)
{
    glm::vec3 corner_pos = glm::vec3(m_chunkCoord) * Constants::CHUNK_WIDTH;
    m_position = corner_pos + (Constants::CHUNK_WIDTH / 2.0f);

    if (m_chunkCoord.y == 0)
    {
        // Generate a simple flat world.
        for (int x = 0; x < Constants::CHUNK_DIM; ++x)
        {
            for (int z = 0; z < Constants::CHUNK_DIM; ++z)
            {
                for (int y = 0; y < Constants::CHUNK_DIM; ++y)
                {
                    if (y < 10)
                        m_blocks[x][y][z] = BlockType::STONE;
                    else if (y < 14)
                        m_blocks[x][y][z] = BlockType::DIRT;
                    else if (y == 14)
                        m_blocks[x][y][z] = BlockType::GRASS;
                    else
                        m_blocks[x][y][z] = BlockType::WATER;
                }
            }
        }
    }
    else
    {
        BlockType fillType = (m_chunkCoord.y < 0) ? BlockType::STONE : BlockType::AIR;
        for (int x = 0; x < Constants::CHUNK_DIM; ++x)
            for (int y = 0; y < Constants::CHUNK_DIM; ++y)
                for (int z = 0; z < Constants::CHUNK_DIM; ++z)
                    m_blocks[x][y][z] = fillType;
    }
}

// Chunk destructor.
Chunk::~Chunk()
{
    delete m_opaqueMesh;
    delete m_transparentMesh;
}

// Returns the block at the given local coordinates.
BlockType Chunk::getBlock(int x, int y, int z) const
{
    if (x < 0 || x >= Constants::CHUNK_DIM || y < 0 || y >= Constants::CHUNK_DIM || z < 0 || z >= Constants::CHUNK_DIM)
        return BlockType::AIR;
    
    return m_blocks[x][y][z];
}

// Generates the mesh for this chunk.
void Chunk::generateMesh(const World &world)
{
    delete m_opaqueMesh;
    m_opaqueMesh = nullptr;
    delete m_transparentMesh;
    m_transparentMesh = nullptr;

    std::vector<Vertex> opaqueVertices;
    std::vector<Vertex> transparentVertices;

    const glm::ivec3 chunkOriginWBC = m_chunkCoord * Constants::CHUNK_DIM;

    for (int d = 0; d < 3; ++d)
    {
        int u = (d + 1) % 3;
        int v = (d + 2) % 3;

        for (int dir = -1; dir <= 1; dir += 2)
        {
            glm::vec3 normal = {0, 0, 0};
            normal[d] = static_cast<float>(dir);

            for (int i = 0; i < Constants::CHUNK_DIM; ++i)
            {
                BlockType mask[Constants::CHUNK_DIM][Constants::CHUNK_DIM] = {{BlockType::AIR}};

                for (int j = 0; j < Constants::CHUNK_DIM; ++j)
                {
                    for (int k = 0; k < Constants::CHUNK_DIM; ++k)
                    {
                        glm::ivec3 localPos(0);
                        localPos[d] = i; localPos[u] = j; localPos[v] = k;

                        const glm::ivec3 currentWBC = chunkOriginWBC + localPos;
                        const glm::ivec3 neighborWBC = currentWBC + glm::ivec3(normal);

                        const BlockType currentType = world.getBlock(currentWBC);
                        const BlockType neighborType = world.getBlock(neighborWBC);

                        const BlockData currentData = Block::get(currentType);
                        const BlockData neighborData = Block::get(neighborType);

                        if (currentType != BlockType::AIR && (neighborType == BlockType::AIR || (!neighborData.isOpaque && currentData.isOpaque)))
                        {
                            mask[j][k] = currentType;
                        }
                    }
                }

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

                        int w = 1;
                        while (k + w < Constants::CHUNK_DIM && mask[j][k + w] == currentType)
                            w++;

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

                        const BlockData blockData = Block::get(currentType);
                        std::vector<Vertex> &targetVertices = blockData.isOpaque ? opaqueVertices : transparentVertices;
                        const glm::vec4 color = blockData.color;

                        glm::vec3 du = {0, 0, 0}, dv = {0, 0, 0};
                        du[u] = 1.0f;
                        dv[v] = 1.0f;

                        glm::vec3 quad_start_pos(0.0f);
                        quad_start_pos[d] = static_cast<float>(i + (dir > 0 ? 1 : 0));
                        quad_start_pos[u] = static_cast<float>(j);
                        quad_start_pos[v] = static_cast<float>(k);

                        glm::vec3 p1 = quad_start_pos;
                        glm::vec3 p2 = quad_start_pos + (static_cast<float>(h) * du);
                        glm::vec3 p3 = quad_start_pos + (static_cast<float>(h) * du) + (static_cast<float>(w) * dv);
                        glm::vec3 p4 = quad_start_pos + (static_cast<float>(w) * dv);

                        const glm::vec3 centerOffset(Constants::CHUNK_DIM / 2.0f);
                        p1 = (p1 - centerOffset) * 0.1f;
                        p2 = (p2 - centerOffset) * 0.1f;
                        p3 = (p3 - centerOffset) * 0.1f;
                        p4 = (p4 - centerOffset) * 0.1f;

                        // Correct winding order for all face directions
                        if (dir > 0)
                        {
                            targetVertices.insert(targetVertices.end(), {{p1, color, normal}, {p2, color, normal}, {p3, color, normal}, {p1, color, normal}, {p3, color, normal}, {p4, color, normal}});
                        }
                        else
                        {
                            targetVertices.insert(targetVertices.end(), {{p1, color, normal}, {p3, color, normal}, {p2, color, normal}, {p1, color, normal}, {p4, color, normal}, {p3, color, normal}});
                        }

                        for (int l = 0; l < h; ++l)
                            for (int m = 0; m < w; ++m)
                                mask[j + l][k + m] = BlockType::AIR;

                        k += w;
                    }
                }
            }
        }
    }

    if (!opaqueVertices.empty())
        m_opaqueMesh = new Mesh(opaqueVertices);

    if (!transparentVertices.empty())
        m_transparentMesh = new Mesh(transparentVertices);
}

// Renders the opaque blocks in this chunk.
void Chunk::renderOpaque(Shader &shader)
{
    if (m_opaqueMesh)
    {
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), m_position);
        shader.setMat4("ModelMatrix", ModelMatrix);
        m_opaqueMesh->render();
    }
}

// Renders the transparent blocks in this chunk.
void Chunk::renderTransparent(Shader &shader)
{
    if (m_transparentMesh)
    {
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), m_position);
        shader.setMat4("ModelMatrix", ModelMatrix);
        m_transparentMesh->render();
    }
}
