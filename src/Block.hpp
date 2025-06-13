#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <map>

enum class BlockType : uint8_t
{
    AIR = 0,
    DIRT,
    GRASS,
    STONE,
    WATER
};

// A simple struct to hold the min/max UVs for one texture in the atlas.
struct TextureCoords {
    glm::vec2 min; // Bottom-left corner
    glm::vec2 max; // Top-right corner
};

struct BlockData
{
    bool isOpaque;
    // Different faces can have different textures.
    TextureCoords side_uvs;
    TextureCoords top_uvs;
    TextureCoords bottom_uvs;
};

// Holds compile-time properties of blocks.
struct BlockTypeData {
    bool isOpaque;
};

class Block
{
public:
    // This map will be populated by the TextureManager at startup.
    static std::map<BlockType, BlockData> block_data_map;

    // Static, compile-time data about block types.
    static const std::map<BlockType, BlockTypeData> blockTypeData;
    
    // The get method is now just a simple map lookup.
    static const BlockData& get(BlockType type)
    {
        // Using .at() will throw an exception if the type is not in the map,
        // which helps debug if a block type was missed during initialization.
        return block_data_map.at(type);
    }

    // Static helper to get compile-time properties.
    static BlockTypeData getProperties(BlockType type)
    {
        auto it = blockTypeData.find(type);
        if (it != blockTypeData.end()) {
            return it->second;
        }
        // Default for AIR or unknown blocks
        return { false };
    }
};
