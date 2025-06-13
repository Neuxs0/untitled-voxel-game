#pragma once

/**
 * @namespace Constants
 * @brief Defines global, compile-time constants for the game.
 *
 * Using a namespace for constants avoids polluting the global namespace
 * and provides clear, scoped access (e.g., Constants::BLOCK_WIDTH).
 */
namespace Constants
{
    // The size of a single block in world units.
    constexpr float BLOCK_SIZE = 1.0f;

    // The number of blocks along one axis of a chunk.
    constexpr int CHUNK_DIM = 16;
    
    // The number of blocks in a 2D slice of a chunk.
    constexpr int CHUNK_AREA = CHUNK_DIM * CHUNK_DIM;

    // The total number of blocks in a chunk's volume.
    constexpr int CHUNK_VOL = CHUNK_DIM * CHUNK_DIM * CHUNK_DIM;

    // The total side length of a chunk in world units.
    constexpr float CHUNK_WIDTH = CHUNK_DIM * BLOCK_SIZE; // 16.0f

    // The player's view distance, in chunks.
    constexpr int RENDER_DISTANCE = 32;
}
