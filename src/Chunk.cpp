#include "Chunk.hpp"
#include "World.hpp"
#include "Shader.hpp"
#include <vector>

void Chunk::calculateAABB()
{
    const float margin = 1.6f;
    const glm::vec3 margin_vec(margin);
    
    m_aabb.min = m_position;
    m_aabb.max = m_position + glm::vec3(Constants::CHUNK_WIDTH);
    m_expandedAabb = {m_aabb.min - margin_vec, m_aabb.max + margin_vec};
}

Chunk::Chunk(glm::ivec3 chunkCoord, const uint32_t *gpuBlockData)
    : m_chunkCoord(chunkCoord)
{
    m_position = glm::vec3(m_chunkCoord) * Constants::CHUNK_WIDTH;
    m_centerPosition = m_position + (Constants::CHUNK_WIDTH / 2.0f);
    calculateAABB();

    for (int x = 0; x < Constants::CHUNK_DIM; ++x)
    {
        for (int y = 0; y < Constants::CHUNK_DIM; ++y)
        {
            for (int z = 0; z < Constants::CHUNK_DIM; ++z)
            {
                int sourceIndex = (x * Constants::CHUNK_AREA) + (y * Constants::CHUNK_DIM) + z;
                m_blocks[x][y][z] = static_cast<BlockType>(gpuBlockData[sourceIndex]);
            }
        }
    }
}

Chunk::~Chunk() {}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    if (x < 0 || x >= Constants::CHUNK_DIM || y < 0 || y >= Constants::CHUNK_DIM || z < 0 || z >= Constants::CHUNK_DIM)
        return BlockType::AIR;
    return m_blocks[x][y][z];
}

MeshResult Chunk::generateMeshStandalone(const World &world) const
{
    MeshResult result;
    result.chunkCoord = m_chunkCoord;
    const glm::ivec3 chunkOriginWBC = m_chunkCoord * Constants::CHUNK_DIM;

    for (int d = 0; d < 3; ++d) 
    {
        int u_axis = (d + 1) % 3; 
        int v_axis = (d + 2) % 3; 

        for (int dir = -1; dir <= 1; dir += 2) 
        {
            glm::vec3 normal = {0, 0, 0};
            normal[d] = static_cast<float>(dir);

            for (int i = 0; i < Constants::CHUNK_DIM; ++i) 
            {
                BlockType mask[Constants::CHUNK_DIM][Constants::CHUNK_DIM] = {{BlockType::AIR}};
                for (int j_mask = 0; j_mask < Constants::CHUNK_DIM; ++j_mask) 
                {
                    for (int k_mask = 0; k_mask < Constants::CHUNK_DIM; ++k_mask) 
                    {
                        glm::ivec3 localPos(0);
                        localPos[d] = i;
                        localPos[u_axis] = j_mask;
                        localPos[v_axis] = k_mask;

                        const glm::ivec3 currentWBC = chunkOriginWBC + localPos;
                        const glm::ivec3 neighborWBC = currentWBC + glm::ivec3(normal);

                        const BlockType currentType = world.getBlock(currentWBC);
                        if (currentType == BlockType::AIR) continue;
                        const BlockType neighborType = world.getBlock(neighborWBC);

                        const BlockTypeData currentProps = Block::getProperties(currentType);
                        const BlockTypeData neighborProps = Block::getProperties(neighborType);
                        
                        bool shouldDrawFace = false;
                        if (currentProps.isOpaque) { 
                            if (!neighborProps.isOpaque) shouldDrawFace = true; 
                        } else { 
                            if (neighborType == BlockType::AIR || neighborProps.isOpaque) {
                                shouldDrawFace = true;
                            } else if (neighborType == currentType) { 
                                shouldDrawFace = false;
                            } else { 
                                shouldDrawFace = true; 
                            }
                        }
                        if(shouldDrawFace) mask[j_mask][k_mask] = currentType;
                    }
                }

                for (int j_quad = 0; j_quad < Constants::CHUNK_DIM; ++j_quad) 
                {
                    for (int k_quad = 0; k_quad < Constants::CHUNK_DIM;) 
                    {
                        BlockType currentType = mask[j_quad][k_quad];
                        if (currentType == BlockType::AIR) {
                            k_quad++;
                            continue;
                        }

                        int quad_width = 1; 
                        while (k_quad + quad_width < Constants::CHUNK_DIM && mask[j_quad][k_quad + quad_width] == currentType)
                            quad_width++;
                        
                        int quad_height = 1; 
                        bool done = false;
                        while (j_quad + quad_height < Constants::CHUNK_DIM && !done) {
                            for (int m = 0; m < quad_width; ++m) {
                                if (mask[j_quad + quad_height][k_quad + m] != currentType) {
                                    done = true;
                                    break;
                                }
                            }
                            if (!done) quad_height++;
                        }

                        const BlockData& blockData = Block::get(currentType);
                        const BlockTypeData& blockTypeProperties = Block::getProperties(currentType);

                        std::vector<Vertex> &targetVertices = blockTypeProperties.isOpaque ? result.opaqueVertices : result.transparentVertices;
                        std::vector<unsigned short> &targetIndices = blockTypeProperties.isOpaque ? result.opaqueIndices : result.transparentIndices;
                        
                        TextureCoords tile_atlas_uvs; // This holds {min_atlas_uv, max_atlas_uv} for the tile
                        if (d == 1) { // Y-face (up/down)
                            tile_atlas_uvs = (dir > 0) ? blockData.top_uvs : blockData.bottom_uvs;
                        } else { // X or Z faces
                            tile_atlas_uvs = blockData.side_uvs;
                        }

                        glm::vec3 quad_start_corner_local(0);
                        quad_start_corner_local[d] = static_cast<float>(i + (dir > 0 ? 1 : 0)); 
                        quad_start_corner_local[u_axis] = static_cast<float>(j_quad);
                        quad_start_corner_local[v_axis] = static_cast<float>(k_quad);

                        glm::vec3 du_vec(0), dv_vec(0); 
                        du_vec[u_axis] = 1.0f;
                        dv_vec[v_axis] = 1.0f;

                        glm::vec3 p0 = quad_start_corner_local;
                        glm::vec3 p1 = quad_start_corner_local + (static_cast<float>(quad_height) * du_vec);
                        glm::vec3 p2 = quad_start_corner_local + (static_cast<float>(quad_height) * du_vec) + (static_cast<float>(quad_width) * dv_vec);
                        glm::vec3 p3 = quad_start_corner_local + (static_cast<float>(quad_width) * dv_vec);
                        
                        unsigned short baseIndex = static_cast<unsigned short>(targetVertices.size());

                        Vertex vert_template;
                        vert_template.normal[0] = static_cast<int8_t>(normal.x);
                        vert_template.normal[1] = static_cast<int8_t>(normal.y);
                        vert_template.normal[2] = static_cast<int8_t>(normal.z);
                        vert_template.color[0] = 255; vert_template.color[1] = 255; vert_template.color[2] = 255; vert_template.color[3] = 255;
                        
                        // Set the atlasOffset (Tx, Ty) for all vertices of this quad
                        // This is the top-left UV coord of the tile within the atlas.
                        vert_template.atlasOffset = tile_atlas_uvs.min;

                        glm::vec2 surface_coords[4];
                        
                        // quad_height is the extent of the quad along the "u" world-axis of the face
                        // quad_width is the extent of the quad along the "v" world-axis of the face
                        float u_extent = static_cast<float>(quad_height);
                        float v_extent = static_cast<float>(quad_width);

                        // Standard UV mapping for quads: (0,0) (1,0) (1,1) (0,1) for TL, TR, BR, BL if U is width and V is height
                        // Our surfaceCoords represent repetition factors (how many times texture repeats)
                        // The orientation of these repetitions depends on the face direction (d) and how du_vec, dv_vec are defined.
                        
                        // For d=0 (X-face), u_axis=Y, v_axis=Z. p0->p1 is along Y (+u_extent), p0->p3 is along Z (+v_extent)
                        // Surface coords: (v, u) -> (width, height) in texture terms typically.
                        // p0: (0,0)
                        // p1: (0, u_extent)  -- along Y
                        // p2: (v_extent, u_extent) -- along Y then Z
                        // p3: (v_extent, 0)  -- along Z
                        surface_coords[0] = glm::vec2(0.0f, 0.0f);
                        surface_coords[1] = glm::vec2(0.0f, u_extent);
                        surface_coords[2] = glm::vec2(v_extent, u_extent);
                        surface_coords[3] = glm::vec2(v_extent, 0.0f);
                        
                        // For d=1 (Y-face), u_axis=Z, v_axis=X. p0->p1 is along Z (+u_extent), p0->p3 is along X (+v_extent)
                        // Similar mapping to X-face.
                        if (d == 1) { // Y-faces (top/bottom)
                            // Surface coords typically (width, depth) or (x, z)
                            // p0: (0,0)
                            // p1: (0, u_extent) -- along Z
                            // p2: (v_extent, u_extent) -- along Z then X
                            // p3: (v_extent, 0) -- along X
                             surface_coords[0] = glm::vec2(0.0f, 0.0f);
                             surface_coords[1] = glm::vec2(0.0f, u_extent); // u_extent is quad_height (along Z)
                             surface_coords[2] = glm::vec2(v_extent, u_extent); // v_extent is quad_width (along X)
                             surface_coords[3] = glm::vec2(v_extent, 0.0f);
                        }

                        // For d=2 (Z-face), u_axis=X, v_axis=Y. p0->p1 is along X (+u_extent), p0->p3 is along Y (+v_extent)
                        // p0: (0,0)
                        // p1: (u_extent, 0) -- along X
                        // p2: (u_extent, v_extent) -- along X then Y
                        // p3: (0, v_extent) -- along Y
                        if (d == 2) {
                             surface_coords[0] = glm::vec2(0.0f, 0.0f);
                             surface_coords[1] = glm::vec2(u_extent, 0.0f);
                             surface_coords[2] = glm::vec2(u_extent, v_extent);
                             surface_coords[3] = glm::vec2(0.0f, v_extent);
                        }


                        vert_template.position = p0; vert_template.surfaceCoords = surface_coords[0]; targetVertices.push_back(vert_template);
                        vert_template.position = p1; vert_template.surfaceCoords = surface_coords[1]; targetVertices.push_back(vert_template);
                        vert_template.position = p2; vert_template.surfaceCoords = surface_coords[2]; targetVertices.push_back(vert_template);
                        vert_template.position = p3; vert_template.surfaceCoords = surface_coords[3]; targetVertices.push_back(vert_template);
                        
                        if (dir > 0) { // Positive face normal
                            targetIndices.insert(targetIndices.end(), {baseIndex, (unsigned short)(baseIndex + 1), (unsigned short)(baseIndex + 2), baseIndex, (unsigned short)(baseIndex + 2), (unsigned short)(baseIndex + 3)});
                        } else { // Negative face normal (flipped winding)
                            targetIndices.insert(targetIndices.end(), {baseIndex, (unsigned short)(baseIndex + 2), (unsigned short)(baseIndex + 1), baseIndex, (unsigned short)(baseIndex + 3), (unsigned short)(baseIndex + 2)});
                        }

                        for (int l = 0; l < quad_height; ++l)
                            for (int m = 0; m < quad_width; ++m)
                                mask[j_quad + l][k_quad + m] = BlockType::AIR;
                        k_quad += quad_width;
                    }
                }
            }
        }
    }
    return result;
}

void Chunk::setOpaqueMeshAllocation(MeshAllocation allocation) { m_opaqueMeshAllocation = allocation; }
void Chunk::setTransparentMeshAllocation(MeshAllocation allocation) { m_transparentMeshAllocation = allocation; }
const glm::vec3 &Chunk::getPosition() const { return m_position; }
const glm::vec3 &Chunk::getCenterPosition() const { return m_centerPosition; }
const AABB &Chunk::getExpandedAABB() const { return m_expandedAabb; }
const MeshAllocation &Chunk::getOpaqueMeshAllocation() const { return m_opaqueMeshAllocation; }
const MeshAllocation &Chunk::getTransparentMeshAllocation() const { return m_transparentMeshAllocation; }
