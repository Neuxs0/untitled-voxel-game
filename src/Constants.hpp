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
    // The number of blocks along one axis of a chunk.
    constexpr int CHUNK_DIM = 16;

    // The total side length of a chunk in world units.
    constexpr float CHUNK_WIDTH = CHUNK_DIM * 0.1f; // 1.6f

    // The player's view distance, in chunks.
    constexpr int RENDER_DISTANCE = 16;
}
