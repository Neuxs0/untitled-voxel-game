#pragma once

#include <string>
#include <GL/glew.h>

namespace ShaderLoader {
    bool loadShadersFromFile(GLuint &program, const std::string &vertShaderPath, const std::string &fragShaderPath);
}
