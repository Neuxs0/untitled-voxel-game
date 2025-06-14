#pragma once

#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <stdexcept>
#include <string_view> // Include string_view

// Represents a shader program.
class Shader
{
private:
    GLuint id;
    mutable std::unordered_map<std::string, GLint> uniformLocationCache;

    // Gets the location of a uniform variable in the shader program.
    GLint getUniformLocation(const std::string &name) const
    {
        if (uniformLocationCache.count(name))
            return uniformLocationCache.at(name);

        GLint location = glGetUniformLocation(id, name.c_str());
        if (location == -1 && id != 0)
            std::cerr << "Error: Shader: getUniformLocation: Uniform '" << name << "' not found in shader program " << id << std::endl;

        uniformLocationCache[name] = location;
        return location;
    }

    // Loads a shader from a source view and returns its handle.
    GLuint loadShader(GLenum type, std::string_view srcView, const char *debugName)
    {
        char infoLog[512];
        GLint successStatus = GL_TRUE;

        GLuint shaderHandle = glCreateShader(type);
        if (shaderHandle == 0)
        {
            std::cerr << "Error: Shader: loadShader: Failed to create shader object for " << debugName << std::endl;
            return 0;
        }

        const char *srcData = srcView.data();
        const GLint srcLength = static_cast<GLint>(srcView.size());

        // Use the glShaderSource overload that accepts a length parameter.
        glShaderSource(shaderHandle, 1, &srcData, &srcLength);

        glCompileShader(shaderHandle);
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &successStatus);
        if (!successStatus)
        {
            glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
            std::cerr << "Error: Shader: loadShader: Couldn't compile shader: " << debugName << std::endl;
            std::cerr << infoLog << std::endl;
            glDeleteShader(shaderHandle);
            return 0;
        }
        return shaderHandle;
    }

    // Links the shaders into a program.
    void linkProgram(GLuint vertShader, GLuint fragShader, GLuint geometryShader)
    {
        char infoLog[512];
        GLint linkSuccess;

        id = glCreateProgram();
        if (id == 0)
            throw std::runtime_error("Error: Shader: linkProgram: Failed to create shader program object.");

        glAttachShader(this->id, vertShader);
        glAttachShader(this->id, fragShader);
        if (geometryShader != 0)
            glAttachShader(this->id, geometryShader);

        glLinkProgram(this->id);
        glGetProgramiv(this->id, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess)
        {
            glGetProgramInfoLog(this->id, 512, NULL, infoLog);
            glDeleteProgram(this->id);
            this->id = 0;
            std::cout << "Error: Shader: linkProgram: Couldn't link program:\n";
            throw std::runtime_error(infoLog);
        }
    }

public:
    // Constructor accepts string_views.
    Shader(std::string_view vertSrc, std::string_view fragSrc, std::string_view geomSrc = {})
        : id(0)
    {
        GLuint vertShader = 0;
        GLuint fragShader = 0;
        GLuint geomShader = 0;

        vertShader = loadShader(GL_VERTEX_SHADER, vertSrc, "VERTEX");
        fragShader = loadShader(GL_FRAGMENT_SHADER, fragSrc, "FRAGMENT");
        if (!geomSrc.empty())
            geomShader = loadShader(GL_GEOMETRY_SHADER, geomSrc, "GEOMETRY");

        if (vertShader == 0 || fragShader == 0)
        {
            if (vertShader != 0)
                glDeleteShader(vertShader);
            if (fragShader != 0)
                glDeleteShader(fragShader);
            if (geomShader != 0)
                glDeleteShader(geomShader);
            throw std::runtime_error("Error: Shader: Failed to load shaders. Shader program not created.");
        }

        linkProgram(vertShader, fragShader, geomShader);

        if (id != 0)
        {
            glDetachShader(id, vertShader);
            glDetachShader(id, fragShader);
            if (geomShader != 0)
                glDetachShader(id, geomShader);
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteShader(geomShader);
    }

    // Destructor.
    ~Shader()
    {
        if (id != 0)
            glDeleteProgram(id);
    }

    // Delete copy constructor and assignment operator.
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    // Move constructor and assignment operator.
    Shader(Shader &&other) noexcept
        : id(other.id), uniformLocationCache(std::move(other.uniformLocationCache))
    {
        other.id = 0;
    }
    Shader &operator=(Shader &&other) noexcept
    {
        if (this != &other)
        {
            if (id != 0)
                glDeleteProgram(id);
            id = other.id;
            uniformLocationCache = std::move(other.uniformLocationCache);
            other.id = 0;
        }
        return *this;
    }

    // Returns the shader program ID.
    inline GLuint getId() const { return id; }
    // Use this shader program.
    void use() const
    {
        if (id != 0)
            glUseProgram(id);
    }
    // Unuse this shader program.
    inline void unuse() const { glUseProgram(0); }
    // Setters for uniform variables.
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
