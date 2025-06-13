#pragma once

#include <string>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp> // Required for glm::vec2
#include "Block.hpp"

class TextureManager {
private:
    GLuint m_atlasTextureID;
    int m_atlasWidth;
    int m_atlasHeight;

    // A map to define which texture files belong to which block.
    // This makes it easy to add new blocks and textures.
    std::map<BlockType, std::string> m_blockTexturePaths = {
        { BlockType::DIRT,  "dirt.png" },
        { BlockType::GRASS, "grass_side.png" }, // Default/side texture
        { BlockType::STONE, "stone.png" },
        { BlockType::WATER, "water.png" } // Although transparent, water still needs a texture
    };
    // Special cases for faces that have different textures.
    std::map<BlockType, std::string> m_topTexturePaths = {
        { BlockType::GRASS, "grass_top.png" }
    };
    std::map<BlockType, std::string> m_bottomTexturePaths = {
        { BlockType::GRASS, "dirt.png" } // Grass block is dirt on the bottom
    };

public:
    TextureManager();
    ~TextureManager();

    // The main function that performs the loading and stitching.
    void loadAndStitch();

    // Getter for the final atlas texture ID.
    GLuint getAtlasID() const { return m_atlasTextureID; }
    // Getters for atlas dimensions
    int getAtlasWidth() const { return m_atlasWidth; }
    int getAtlasHeight() const { return m_atlasHeight; }
    // Getter for normalized tile size in the atlas
    glm::vec2 getNormalizedTileSize() const;
};
