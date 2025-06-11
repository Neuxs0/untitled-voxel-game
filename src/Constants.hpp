#pragma once

namespace Constants
{
    // The final size of one voxel
    constexpr float BLOCK_WIDTH = 1.0f / 10.0f; // 10cm

    // The number of blocks along one axis of a chunk
    constexpr int CHUNK_DIM = 16;

    // The total size of one chunk
    constexpr float CHUNK_WIDTH = CHUNK_DIM * BLOCK_WIDTH; // This will be 1.0f
}
