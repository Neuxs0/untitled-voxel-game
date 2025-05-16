#include "shader_loader.h"
#include "utils/files.h"

#include <iostream>

namespace ShaderLoader {

bool loadShadersFromFile(GLuint &program, const std::string &vertShaderPath, const std::string &fragShaderPath) {
    char infoLog[512];
    GLint compile_success;
    bool fileReadSuccess = true;
    std::string src;

    // Vertex
    src = Utils::readFileToString(vertShaderPath, fileReadSuccess);
    if (!fileReadSuccess) {
        std::cerr << "Error:ShaderLoader:loadShadersFromFile: Failed to read vertex shader file: " << vertShaderPath << "\n";
        return false;
    }

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertSrc = src.c_str();
    glShaderSource(vertShader, 1, &vertSrc, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compile_success);
    if (!compile_success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        std::cerr << "Error:ShaderLoader:loadShadersFromFile: Couldn't compile vertex shader: " << vertShaderPath << "\n";
        std::cerr << infoLog << "\n";
        glDeleteShader(vertShader);
        return false;
    }

    // Fragment
    src = Utils::readFileToString(fragShaderPath, fileReadSuccess);
    if (!fileReadSuccess) {
        std::cerr << "Error:ShaderLoader:loadShadersFromFile: Failed to read fragment shader file: " << fragShaderPath << "\n";
        glDeleteShader(vertShader);
        return false;
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragSrc = src.c_str();
    glShaderSource(fragShader, 1, &fragSrc, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compile_success);
    if (!compile_success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cerr << "Error:ShaderLoader:loadShadersFromFile: Couldn't compile fragment shader: " << fragShaderPath << "\n";
        std::cerr << infoLog << "\n";
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return false;
    }

    // Program
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint link_success;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if (!link_success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Error:ShaderLoader:loadShadersFromFile: Couldn't link program" << "\n";
        std::cerr << infoLog << "\n";
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    // Detach and delete shaders
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return true;
}

} // namespace ShaderLoader
