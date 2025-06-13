#version 460 core
layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// Constants
const int CHUNK_DIM = 16;
const uint AIR = 0u;
const uint DIRT = 1u;
const uint GRASS = 2u;

// The buffer to store the generated block data.
// It's a 1D array representing the 3D chunk volume.
layout(std430, binding = 0) buffer BlockBuffer {
    uint blocks[];
};

// Uniforms
uniform ivec3 u_chunkCoord; // The coordinate of the chunk being generated.

void main() {
    // Get the 3D local coordinate of this thread within the work group.
    ivec3 localPos = ivec3(gl_GlobalInvocationID);

    // Make sure we're not trying to write outside the chunk's boundaries.
    if (any(greaterThanEqual(localPos, ivec3(CHUNK_DIM)))) {
        return;
    }

    // Calculate the 1D index for the `blocks` buffer from the 3D local position.
    uint index = localPos.x * CHUNK_DIM * CHUNK_DIM + localPos.y * CHUNK_DIM + localPos.z;

    // Calculate the world coordinate of the block. We use the center for noise sampling.
    ivec3 worldBlockCoord_minCorner = u_chunkCoord * CHUNK_DIM + localPos;
    float worldX_center = float(worldBlockCoord_minCorner.x) + 0.5f;
    int   worldY_block  = worldBlockCoord_minCorner.y; 
    float worldZ_center = float(worldBlockCoord_minCorner.z) + 0.5f;

    // Determine the terrain height at this X, Z position using a simple noise function.
    // This creates a basic, rolling-hills landscape.
    float amplitude = 10.0f;
    float frequency = 0.1f;
    int base_height_block_y = 0;
    // Combine sine and cosine to create a more varied pattern than a simple wave.
    int terrain_surface_y = base_height_block_y + int(amplitude * (sin(worldX_center * frequency) + cos(worldZ_center * frequency)) / 2.0f);

    // Determine the block type based on its height relative to the surface.
    uint blockID = AIR;
    if (worldY_block == terrain_surface_y) {
        blockID = GRASS;
    } else if (worldY_block < terrain_surface_y && worldY_block >= terrain_surface_y - 3) {
        // Place a few layers of dirt below the grass.
        blockID = DIRT;
    } else {
        // Everything else is air.
        blockID = AIR; 
    }
    
    // Write the final block ID to the buffer.
    blocks[index] = blockID;
}
