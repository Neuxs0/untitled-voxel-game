#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include "TextureManager.hpp"
#include "EmbeddedTextures.hpp" 
#include "Constants.hpp" 
#include <cmath>
#include <iostream>
#include <iomanip> 
#include <set>
#include <vector> 
#include <algorithm> 

// Constructor
TextureManager::TextureManager() : m_atlasTextureID(0), m_atlasWidth(0), m_atlasHeight(0) {}

// Destructor
TextureManager::~TextureManager() {
    if (m_atlasTextureID != 0) {
        glDeleteTextures(1, &m_atlasTextureID);
    }
}

void TextureManager::loadAndStitch() {
    struct RawImage {
        std::string name;
        int width, height;
        unsigned char* data;
    };
    std::vector<RawImage> loadedImages;
    std::set<std::string> uniqueTexturePaths; 

    stbi_set_flip_vertically_on_load(true);

    for(auto const& [key, val] : m_blockTexturePaths) uniqueTexturePaths.insert(val);
    for(auto const& [key, val] : m_topTexturePaths) uniqueTexturePaths.insert(val);
    for(auto const& [key, val] : m_bottomTexturePaths) uniqueTexturePaths.insert(val);
    
    for (const auto& path : uniqueTexturePaths) {
        auto textureDataOpt = EmbeddedTextures::get(path);
        if (!textureDataOpt) {
            std::cerr << "TextureManager Error: Failed to find embedded texture: " << path << std::endl;
            continue;
        }
        std::string_view textureData = *textureDataOpt;
        
        int w, h, channels;
        unsigned char* data = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(textureData.data()),
            static_cast<int>(textureData.size()),
            &w, &h, &channels, 4); 

        if (data) {
            if (w != Constants::TEXTURE_SIZE_PX || h != Constants::TEXTURE_SIZE_PX) {
                 std::cerr << "TextureManager Warning: Texture '" << path << "' is " << w << "x" << h 
                           << " but expected " << Constants::TEXTURE_SIZE_PX << "x" << Constants::TEXTURE_SIZE_PX 
                           << ". Atlas packing might be incorrect." << std::endl;
            }
            loadedImages.push_back({path, w, h, data});
        } else {
            std::cerr << "TextureManager Error: Failed to decode embedded texture: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
        }
    }

    if (loadedImages.empty()) {
        std::cerr << "TextureManager Error: No textures were loaded. Cannot create atlas." << std::endl;
        return;
    }

    const int texSize = Constants::TEXTURE_SIZE_PX; 
    int texturesPerRow = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(loadedImages.size())))));
    m_atlasWidth = texturesPerRow * texSize;
    m_atlasHeight = static_cast<int>(std::ceil(static_cast<double>(loadedImages.size()) / texturesPerRow)) * texSize;
    if (m_atlasHeight == 0 && !loadedImages.empty()) { 
        m_atlasHeight = texSize;
    }

    std::cout << "TextureManager: Atlas dimensions: " << m_atlasWidth << "x" << m_atlasHeight << " (" << texturesPerRow << " textures per row, " << loadedImages.size() << " total textures)" << std::endl;

    glGenTextures(1, &m_atlasTextureID);
    glBindTexture(GL_TEXTURE_2D, m_atlasTextureID);

    std::vector<unsigned char> atlas_initial_data(m_atlasWidth * m_atlasHeight * 4, 0);
    for(size_t i = 3; i < atlas_initial_data.size(); i += 4) { 
        atlas_initial_data[i] = 0;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_atlasWidth, m_atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_initial_data.data());

    std::map<std::string, TextureCoords> uvMap;
    int currentX = 0;
    int currentY = 0;

    for (const auto& img : loadedImages) {
        if (currentX + img.width > m_atlasWidth) { 
            currentX = 0;
            currentY += texSize; 
        }
        if (currentY + img.height > m_atlasHeight) {
            std::cerr << "TextureManager Error: Not enough space in atlas for texture " << img.name 
                      << ". Atlas: " << m_atlasWidth << "x" << m_atlasHeight 
                      << ", Img: " << img.width << "x" << img.height << " at " << currentX << "," << currentY << std::endl;
            stbi_image_free(img.data);
            continue; 
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, img.width, img.height, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
        
        uvMap[img.name] = {
            .min = glm::vec2(static_cast<float>(currentX) / m_atlasWidth,
                             static_cast<float>(currentY) / m_atlasHeight),
            .max = glm::vec2(static_cast<float>(currentX + img.width) / m_atlasWidth,
                             static_cast<float>(currentY + img.height) / m_atlasHeight)
        };
        currentX += texSize; 
        stbi_image_free(img.data);
    }

    // Set texture filtering to NEAREST for a pixelated art style. Mipmaps are not used.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    for(auto const& [type, blockTypeProperties] : Block::blockTypeData) {
        auto path_it = m_blockTexturePaths.find(type);
        if(path_it == m_blockTexturePaths.end()) {
            if (type != BlockType::AIR) { }
            continue;
        }

        BlockData data;
        data.isOpaque = blockTypeProperties.isOpaque; 
        
        const std::string& side_texture_filename = path_it->second;
        if (uvMap.count(side_texture_filename)) {
            data.side_uvs = uvMap.at(side_texture_filename);
        } else {
            std::cerr << "TextureManager Error: Missing UVs for side texture '" << side_texture_filename << "' for BlockType " << static_cast<int>(type) << std::endl;
            data.side_uvs = {}; 
        }
        
        data.top_uvs = data.side_uvs; 
        auto top_path_it = m_topTexturePaths.find(type);
        if (top_path_it != m_topTexturePaths.end()) {
            const std::string& top_texture_filename = top_path_it->second;
            if (uvMap.count(top_texture_filename)) {
                data.top_uvs = uvMap.at(top_texture_filename);
            } else {
                 std::cerr << "TextureManager Error: Missing UVs for top texture '" << top_texture_filename << "' for BlockType " << static_cast<int>(type) << std::endl;
            }
        }
        
        data.bottom_uvs = data.side_uvs; 
        auto bottom_path_it = m_bottomTexturePaths.find(type);
        if (bottom_path_it != m_bottomTexturePaths.end()) {
            const std::string& bottom_texture_filename = bottom_path_it->second;
            if(uvMap.count(bottom_texture_filename)) {
                data.bottom_uvs = uvMap.at(bottom_texture_filename);
            } else {
                std::cerr << "TextureManager Error: Missing UVs for bottom texture '" << bottom_texture_filename << "' for BlockType " << static_cast<int>(type) << std::endl;
            }
        }
        Block::block_data_map[type] = data;
    }
    Block::block_data_map[BlockType::AIR] = { .isOpaque = false, .side_uvs = {}, .top_uvs = {}, .bottom_uvs = {} };
}

glm::vec2 TextureManager::getNormalizedTileSize() const {
    if (m_atlasWidth > 0 && m_atlasHeight > 0) {
        return glm::vec2(static_cast<float>(Constants::TEXTURE_SIZE_PX) / static_cast<float>(m_atlasWidth),
                         static_cast<float>(Constants::TEXTURE_SIZE_PX) / static_cast<float>(m_atlasHeight));
    }
    std::cerr << "Warning: Atlas dimensions are zero in getNormalizedTileSize." << std::endl;
    return glm::vec2(0.0f); 
}
