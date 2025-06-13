#include "EmbeddedTextures.hpp"
#include <map>
#include <string>
#include <string_view>

// --- Linker Symbols for Embedded Textures ---
// The linker generates these symbols from the file paths.
// The format is: _binary_PATH_WITH_UNDERSCORES_start/end
extern const char _binary_src_assets_textures_blocks_dirt_png_start[];
extern const char _binary_src_assets_textures_blocks_dirt_png_end[];
extern const char _binary_src_assets_textures_blocks_grass_side_png_start[];
extern const char _binary_src_assets_textures_blocks_grass_side_png_end[];
extern const char _binary_src_assets_textures_blocks_grass_top_png_start[];
extern const char _binary_src_assets_textures_blocks_grass_top_png_end[];
extern const char _binary_src_assets_textures_blocks_stone_png_start[];
extern const char _binary_src_assets_textures_blocks_stone_png_end[];
extern const char _binary_src_assets_textures_blocks_water_png_start[];
extern const char _binary_src_assets_textures_blocks_water_png_end[];

// Use a static map to store the mapping from filename to data.
static std::map<std::string, std::string_view> textureDataMap;
static bool isInitialized = false;

// Initialize the map with data from the linker symbols.
void EmbeddedTextures::initialize()
{
    if (isInitialized) return;

    textureDataMap["dirt.png"] = std::string_view(
        _binary_src_assets_textures_blocks_dirt_png_start,
        _binary_src_assets_textures_blocks_dirt_png_end - _binary_src_assets_textures_blocks_dirt_png_start
    );
    textureDataMap["grass_side.png"] = std::string_view(
        _binary_src_assets_textures_blocks_grass_side_png_start,
        _binary_src_assets_textures_blocks_grass_side_png_end - _binary_src_assets_textures_blocks_grass_side_png_start
    );
    textureDataMap["grass_top.png"] = std::string_view(
        _binary_src_assets_textures_blocks_grass_top_png_start,
        _binary_src_assets_textures_blocks_grass_top_png_end - _binary_src_assets_textures_blocks_grass_top_png_start
    );
    textureDataMap["stone.png"] = std::string_view(
        _binary_src_assets_textures_blocks_stone_png_start,
        _binary_src_assets_textures_blocks_stone_png_end - _binary_src_assets_textures_blocks_stone_png_start
    );
    textureDataMap["water.png"] = std::string_view(
        _binary_src_assets_textures_blocks_water_png_start,
        _binary_src_assets_textures_blocks_water_png_end - _binary_src_assets_textures_blocks_water_png_start
    );

    isInitialized = true;
}

std::optional<std::string_view> EmbeddedTextures::get(const std::string& filename) {
    if (!isInitialized) {
        initialize();
    }

    auto it = textureDataMap.find(filename);
    if (it != textureDataMap.end()) {
        return it->second;
    }

    return std::nullopt;
}
