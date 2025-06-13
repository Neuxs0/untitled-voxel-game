#include "Block.hpp"

// Definition for the static block data map.
// This will be populated by the TextureManager at startup.
std::map<BlockType, BlockData> Block::block_data_map;

// Definition for the static, compile-time block property data.
const std::map<BlockType, BlockTypeData> Block::blockTypeData = {
    {BlockType::AIR, {false}},
    {BlockType::DIRT, {true}},
    {BlockType::GRASS, {true}},
    {BlockType::STONE, {true}},
    {BlockType::WATER, {false}}
};
