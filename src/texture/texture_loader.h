#pragma once

#include <GL/glew.h>
#include <string>

namespace TextureLoader
{
    bool loadTexture(const char* filePath, GLuint& textureID, int& width, int& height);
}
