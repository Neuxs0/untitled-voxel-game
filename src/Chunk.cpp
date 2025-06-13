#include "Chunk.hpp"
#include "World.hpp"
#include "Shader.hpp"
#include <vector>

void Chunk::calculateAABB() {
    const float margin = 1.6f;
    const glm::vec3 margin_vec(margin);
    float halfWidth = Constants::CHUNK_WIDTH / 2.0f;
    glm::vec3 halfSize(halfWidth);
    m_aabb = {m_position - halfSize, m_position + halfSize};
    m_expandedAabb = {m_aabb.min - margin_vec, m_aabb.max + margin_vec};
}

Chunk::Chunk(glm::ivec3 chunkCoord, const uint32_t *gpuBlockData)
    : m_chunkCoord(chunkCoord) {
    glm::vec3 corner_pos = glm::vec3(m_chunkCoord) * Constants::CHUNK_WIDTH;
    m_position = corner_pos + (Constants::CHUNK_WIDTH / 2.0f);
    calculateAABB();

    for (int x = 0; x < Constants::CHUNK_DIM; ++x) {
        for (int y = 0; y < Constants::CHUNK_DIM; ++y) {
            for (int z = 0; z < Constants::CHUNK_DIM; ++z) {
                int sourceIndex = (x * Constants::CHUNK_VOL / Constants::CHUNK_DIM) + (y * Constants::CHUNK_AREA / Constants::CHUNK_DIM) + z;
                m_blocks[x][y][z] = static_cast<BlockType>(gpuBlockData[sourceIndex]);
            }
        }
    }
}

Chunk::~Chunk() {}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= Constants::CHUNK_DIM || y < 0 || y >= Constants::CHUNK_DIM || z < 0 || z >= Constants::CHUNK_DIM)
        return BlockType::AIR;
    return m_blocks[x][y][z];
}

MeshResult Chunk::generateMeshStandalone(const World &world) const {
    MeshResult result;
    result.chunkCoord = m_chunkCoord;
    const glm::ivec3 chunkOriginWBC = m_chunkCoord * Constants::CHUNK_DIM;

    // Iterate over each of the 3 dimensions (X, Y, Z)
    for (int d = 0; d < 3; ++d) {
        // Define the other two dimensions for the 2D slice
        int u = (d + 1) % 3;
        int v = (d + 2) % 3;
        // Iterate over both directions along the current dimension (e.g., +X and -X)
        for (int dir = -1; dir <= 1; dir += 2) {
            glm::vec3 normal = {0, 0, 0};
            normal[d] = static_cast<float>(dir);
            // Process the chunk slice by slice along the current dimension
            for (int i = 0; i < Constants::CHUNK_DIM; ++i) {
                // Create a 2D mask to store block types for the current slice
                BlockType mask[Constants::CHUNK_DIM][Constants::CHUNK_DIM] = {{BlockType::AIR}};
                // Populate the mask with faces that should be rendered
                for (int j = 0; j < Constants::CHUNK_DIM; ++j) {
                    for (int k = 0; k < Constants::CHUNK_DIM; ++k) {
                        // Current block's local and world coordinates
                        glm::ivec3 localPos(0);
                        localPos[d] = i; localPos[u] = j; localPos[v] = k;
                        const glm::ivec3 currentWBC = chunkOriginWBC + localPos;
                        // Neighbor block's world coordinates (in the direction of the face normal)
                        const glm::ivec3 neighborWBC = currentWBC + glm::ivec3(normal);
                        // Get block types and data for current and neighbor blocks
                        const BlockType currentType = world.getBlock(currentWBC);
                        const BlockType neighborType = world.getBlock(neighborWBC);
                        const BlockData currentData = Block::get(currentType);
                        const BlockData neighborData = Block::get(neighborType);
                        // A face is visible if the current block is solid and the neighbor is transparent/air.
                        if (currentType != BlockType::AIR && (neighborType == BlockType::AIR || (!neighborData.isOpaque && currentData.isOpaque))) {
                            mask[j][k] = currentType;
                        }
                    }
                }
                // Generate quads from the 2D mask using greedy meshing
                for (int j = 0; j < Constants::CHUNK_DIM; ++j) {
                    for (int k = 0; k < Constants::CHUNK_DIM;) {
                        BlockType currentType = mask[j][k];
                        if (currentType == BlockType::AIR) { k++; continue; }
                        // Find the width of the quad
                        int w = 1;
                        while (k + w < Constants::CHUNK_DIM && mask[j][k + w] == currentType) w++;
                        // Find the height of the quad
                        int h = 1; bool done = false;
                        while (j + h < Constants::CHUNK_DIM && !done) {
                            for (int m = 0; m < w; ++m) {
                                if (mask[j + h][k + m] != currentType) { done = true; break; }
                            }
                            if (!done) h++;
                        }
                        // Get block data to determine which mesh (opaque/transparent) to use
                        const BlockData blockData = Block::get(currentType);
                        std::vector<Vertex> &targetVertices = blockData.isOpaque ? result.opaqueVertices : result.transparentVertices;
                        std::vector<unsigned int> &targetIndices = blockData.isOpaque ? result.opaqueIndices : result.transparentIndices;
                        // Define quad vertices based on position, width, and height
                        glm::vec3 du = {0,0,0}, dv = {0,0,0}; du[u] = 1; dv[v] = 1;
                        glm::vec3 quad_start_pos(0);
                        quad_start_pos[d] = i + (dir > 0); quad_start_pos[u] = j; quad_start_pos[v] = k;
                        glm::vec3 p1 = quad_start_pos;
                        glm::vec3 p2 = quad_start_pos + (float(h) * du);
                        glm::vec3 p3 = quad_start_pos + (float(h) * du) + (float(w) * dv);
                        glm::vec3 p4 = quad_start_pos + (float(w) * dv);
                        // Center the vertices around the chunk's origin for model matrix transformation
                        const glm::vec3 centerOffset(Constants::CHUNK_DIM / 2.0f);
                        p1 = (p1-centerOffset); p2 = (p2-centerOffset); p3 = (p3-centerOffset); p4 = (p4-centerOffset);
                        // Add the four vertices to the target vertex list
                        unsigned int baseIndex = targetVertices.size();
                        Vertex v_template;
                        v_template.color[0] = blockData.color.r * 255.0f; v_template.color[1] = blockData.color.g * 255.0f;
                        v_template.color[2] = blockData.color.b * 255.0f; v_template.color[3] = blockData.color.a * 255.0f;
                        v_template.normal[0] = normal.x * 127; v_template.normal[1] = normal.y * 127; v_template.normal[2] = normal.z * 127;
                        v_template.position = p1; targetVertices.push_back(v_template);
                        v_template.position = p2; targetVertices.push_back(v_template);
                        v_template.position = p3; targetVertices.push_back(v_template);
                        v_template.position = p4; targetVertices.push_back(v_template);
                        // Add indices for the quad, handling winding order based on face direction
                        if (dir > 0) targetIndices.insert(targetIndices.end(), { baseIndex, baseIndex+1, baseIndex+2, baseIndex, baseIndex+2, baseIndex+3 });
                        else targetIndices.insert(targetIndices.end(), { baseIndex, baseIndex+2, baseIndex+1, baseIndex, baseIndex+3, baseIndex+2 });
                        // Zero out the mask for the area covered by the new quad
                        for (int l = 0; l < h; ++l) for (int m = 0; m < w; ++m) mask[j+l][k+m] = BlockType::AIR;
                        // Move to the next unchecked part of the mask
                        k += w;
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
const AABB &Chunk::getExpandedAABB() const { return m_expandedAabb; }
const MeshAllocation &Chunk::getOpaqueMeshAllocation() const { return m_opaqueMeshAllocation; }
const MeshAllocation &Chunk::getTransparentMeshAllocation() const { return m_transparentMeshAllocation; }
