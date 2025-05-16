#include "texture_loader.h"

#include <soil2/SOIL2.h>
#include <iostream>

namespace TextureLoader {

bool loadTexture(const char* filePath, GLuint& textureID, int& texWidth, int& texHeight) {
    unsigned char* image_data = SOIL_load_image(filePath, &texWidth, &texHeight, NULL, SOIL_LOAD_RGBA);

    if (!image_data) {
        std::cerr << "Error:TextureLoader: Failed to load texture '" << filePath << "': " << SOIL_last_result() << std::endl;
        return false;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Load texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Free image data
    SOIL_free_image_data(image_data);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Info:TextureLoader: Successfully loaded texture '" << filePath << "' (ID: " << textureID << ")" << std::endl;

    return true;
}

} // namespace TextureLoader
