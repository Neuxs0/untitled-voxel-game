#pragma once

#include <cstdint>
#include <glm/glm.hpp>

enum class BlockType : uint8_t
{
    AIR = 0,
    DIRT,
    GRASS,
    STONE,
    WATER
};

struct BlockData
{
    glm::vec4 color;
    bool isOpaque;
};

class Block
{
public:
    static BlockData get(BlockType type)
    {
        switch (type)
        {
        // Note: 'isOpaque' must be true for solid blocks to ensure correct face culling and lighting.
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
