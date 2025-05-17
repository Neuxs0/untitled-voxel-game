#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils/files.h"

class Shader
{

private:
    GLuint id;
    mutable std::unordered_map<std::string, GLint> uniformLocationCache;

    GLint getUniformLocation(const std::string &name) const
    {
        if (uniformLocationCache.count(name))
            return uniformLocationCache.at(name);
        
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location == -1)
            if (id != 0)
                std::cerr << "Error: Shader: getUniformLocation: Uniform '" << name << "' not found in shader program " << id << std::endl;
        
        uniformLocationCache[name] = location;
        return location;
    }

    std::string readShader(const std::string &fileName)
    {
        bool success = true;
        std::string path = "assets/shaders/" + fileName;
        std::string src = Utils::readFile(path, success);
        if (!success)
        {
            std::cerr << "Error: Shader: readShader: Couldn't read shader file: " << path << std::endl;
            return "";
        }
        if (src.empty())
        {
            std::cerr << "Error: Shader: readShader: Shader file " << path << " is empty." << std::endl;
            return "";
        }
        return src;
    }

    GLuint loadShader(GLenum type, const std::string &fileName)
    {
        char infoLog[512];
        GLint successStatus = GL_TRUE;

        GLuint shaderHandle = glCreateShader(type);
        if (shaderHandle == 0)
        {
            std::cerr << "Error: Shader: loadShader: Failed to create shader object for " << fileName << std::endl;
            return 0;
        }

        std::string srcStr = this->readShader(fileName);

        if (srcStr.empty())
        {
            std::cerr << "Error: Shader: loadShader: Source code for shader " << fileName << " is empty or unreadable." << std::endl;
            glDeleteShader(shaderHandle);
            return 0;
        }

        const GLchar *src = srcStr.c_str();
        glShaderSource(shaderHandle, 1, &src, NULL);
        glCompileShader(shaderHandle);
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &successStatus);

        if (!successStatus)
        {
            glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
            std::cerr << "Error: Shader: loadShader: Couldn't compile shader: " << fileName << std::endl;
            std::cerr << infoLog << std::endl;
            glDeleteShader(shaderHandle);
            return 0;
        }
        return shaderHandle;
    }

    void linkProgram(GLuint vertShader, GLuint fragShader, GLuint geometryShader)
    {
        char infoLog[512];
        GLint linkSuccess;

        this->id = glCreateProgram();
        if (this->id == 0)
        {
            std::cerr << "Error: Shader: linkProgram: Failed to create shader program object." << std::endl;
            return;
        }

        // Attach shaders
        glAttachShader(this->id, vertShader);
        glAttachShader(this->id, fragShader);
        if (geometryShader != 0)
            glAttachShader(this->id, geometryShader);

        glLinkProgram(this->id);

        glGetProgramiv(this->id, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess)
        {
            glGetProgramInfoLog(this->id, 512, NULL, infoLog);
            std::cerr << "Error: Shader: linkProgram: Couldn't link program:" << std::endl
                      << infoLog << std::endl;
            glDeleteProgram(this->id);
            this->id = 0;
        }
    }

public:
    // Constructor
    Shader(const std::string &vertFile, const std::string &fragFile, const std::string *geomFile = nullptr)
        : id(0)
    {
        GLuint vertShader = 0;
        GLuint fragShader = 0;
        GLuint geomShader = 0;

        vertShader = loadShader(GL_VERTEX_SHADER, vertFile);
        fragShader = loadShader(GL_FRAGMENT_SHADER, fragFile);

        if (geomFile && !geomFile->empty())
        {
            geomShader = loadShader(GL_GEOMETRY_SHADER, *geomFile);
        }

        if (vertShader == 0 || fragShader == 0)
        {
            std::cerr << "Error: Shader: Constructor: Failed to load shaders. Shader program not created." << std::endl;
            
            if (vertShader != 0)
                glDeleteShader(vertShader);
            if (fragShader != 0)
                glDeleteShader(fragShader);
            if (geomShader != 0)
                glDeleteShader(geomShader);
            
            return;
        }

        this->linkProgram(vertShader, fragShader, geomShader);

        // Cleanup shaders
        if (this->id != 0)
        {
            glDetachShader(this->id, vertShader);
            glDetachShader(this->id, fragShader);
            if (geomShader != 0)
            {
                glDetachShader(this->id, geomShader);
            }
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteShader(geomShader);
    }

    // Destructor
    ~Shader()
    {
        if (id != 0)
            glDeleteProgram(id);
    }

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&other) noexcept
        : id(other.id), uniformLocationCache(std::move(other.uniformLocationCache))
    {
        other.id = 0;
    }

    // Move assignment operator
    Shader &operator=(Shader &&other) noexcept
    {
        if (this != &other)
        {
            if (id != 0)
            {
                glDeleteProgram(id);
            }
            id = other.id;
            uniformLocationCache = std::move(other.uniformLocationCache);
            other.id = 0;
        }
        return *this;
    }

    GLuint getId() const { return this->id; }

    void use() const
    {
        if (id != 0)
            glUseProgram(this->id);
    }
    void unuse() const { glUseProgram(0); }

    // Setters
    void setBool(const std::string &name, bool value) const
    {
        if (id != 0)
            glUniform1i(getUniformLocation(name), (int)value);
    }
    void setInt(const std::string &name, int value) const
    {
        if (id != 0)
            glUniform1i(getUniformLocation(name), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        if (id != 0)
            glUniform1f(getUniformLocation(name), value);
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const
    {
        if (id != 0)
            glUniform2fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const
    {
        if (id != 0)
            glUniform3fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec4(const std::string &name, const glm::vec4 &value) const
    {
        if (id != 0)
            glUniform4fv(getUniformLocation(name), 1, &value[0]);
    }
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        if (id != 0)
            glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        if (id != 0)
            glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
};
