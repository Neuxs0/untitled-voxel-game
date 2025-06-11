#pragma once

#include <cstdint>
#include <glm/glm.hpp>

// An enumeration of all possible block types.
enum class BlockType : uint8_t
{
    AIR = 0,
    DIRT,
    GRASS,
    STONE,
    WATER
};

// A struct containing the data for a block.
struct BlockData
{
    glm::vec4 color;
    bool isOpaque;
};

// A static class for getting block data.
class Block
{
public:
    // Returns the data for a given block type.
    static BlockData get(BlockType type)
    {
        switch (type)
        {
        case BlockType::DIRT:
            return {{0.54f, 0.27f, 0.07f, 1.0f}, true};
        case BlockType::GRASS:
            return {{0.1f, 0.6f, 0.2f, 1.0f}, true};
        case BlockType::STONE:
            return {{0.5f, 0.5f, 0.5f, 1.0f}, true};
        case BlockType::WATER:
            return {{0.1f, 0.3f, 0.8f, 0.5f}, false};
        default:
            return {{0.0f, 0.0f, 0.0f, 0.0f}, false};
        }
    }
};
