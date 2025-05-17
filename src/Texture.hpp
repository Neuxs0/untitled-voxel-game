#pragma once

#include <iostream>
#include <GL/glew.h>

#include <soil2/SOIL2.h>

class Texture
{
private:
    GLuint id;
    int width;
    int height;
    GLenum type;
    GLint texUnit;

    unsigned char *loadImage(const std::string &fileName)
    {
        std::string path = "assets/textures/" + fileName;
        unsigned char *image = SOIL_load_image(path.c_str(), &width, &height, NULL, SOIL_LOAD_RGBA);
        if (!image)
            throw std::runtime_error("Error: Texture: loadImage: Failed to load texture " + path + ": " + SOIL_last_result());
        return image;
    }

public:
    Texture(const std::string &fileName, const GLint &texUnit, const GLenum &type = GL_TEXTURE_2D)
        : id(0), width(0), height(0), type(type), texUnit(texUnit)
    {
        if (id != 0)
            glDeleteTextures(1, &id);

        unsigned char *image = loadImage(fileName);
        
        // Generate texture
        glGenTextures(1, &id);
        glBindTexture(type, id);

        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR)
            throw std::runtime_error("OpenGL error: " + std::to_string(err));

        // Set texture wrapping parameters
        glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // Set texture filtering parameters
        glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        
        // Load texture data
        glTexImage2D(type, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(type);
        
        // Free image data
        SOIL_free_image_data(image);
        
        // Unbind texture
        glBindTexture(type, 0);
    }

    ~Texture()
    {
        glDeleteTextures(1, &id);
    }

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&other) noexcept
        : id(other.id), width(other.width), height(other.height), type(other.type), texUnit(other.texUnit)
    {
        other.id = 0;
    }
    Texture &operator=(Texture &&other) noexcept
    {
        if (this != &other)
        {
            if (id != 0)
                glDeleteTextures(1, &id);

            // Transfer ownership
            id = other.id;
            width = other.width;
            height = other.height;
            type = other.type;
            texUnit = other.texUnit;

            other.id = 0;
        }
        return *this;
    }

    inline GLuint getId() const { return id; }
    inline GLint getTexUnit() const { return texUnit; }

    void bind()
    {
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(type, id);
    }
    void unbind()
    {
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(type, 0);
    }
};
