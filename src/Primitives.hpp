#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Vertex.hpp"


class Primitive
{
private:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;


public:
    Primitive() {}
    virtual ~Primitive() {}

    void set(Vertex *vertices, const unsigned numVertices, GLuint *indices, const unsigned numIndices)
    {
        for (size_t i = 0; i < numVertices; i++)
        {
            this->vertices.push_back(vertices[i]);
        }

        for (size_t i = 0; i < numIndices; i++)
        {
            this->indices.push_back(indices[i]);
        }
    }

    inline Vertex *getVertices() { return vertices.data(); }
    inline GLuint *getIndices() { return indices.data(); }
    inline unsigned getNumVertices() { return vertices.size(); }
    inline unsigned getNumIndices() { return indices.size(); }
};

class Quad : public Primitive
{
public:
    Quad() : Primitive()
    {
        Vertex vertices[] =
            {
                // position                     // color                     // texcoord            // normal
                {glm::vec3(-0.5f, 0.5f, 0.5f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
                {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
                {glm::vec3(0.5f, -0.5f, 0.5f),  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
                {glm::vec3(0.5f, 0.5f, 0.5f),   glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
            };
        unsigned numVertices = sizeof(vertices) / sizeof(Vertex);

        GLuint indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };
        unsigned numIndices = sizeof(indices) / sizeof(GLuint);

        set(vertices, numVertices, indices, numIndices);
    }
};

class Cube : public Primitive
{
public:
    Cube() : Primitive()
    {
        Vertex vertices[] =
        {
            // Front face (+Z)
            {glm::vec3(-0.5f, 0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},   // Top-Left
            {glm::vec3(-0.5f, -0.5f, 0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},   // Bottom-Left
            {glm::vec3(0.5f, -0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},   // Bottom-Right
            {glm::vec3(0.5f, 0.5f, 0.5f),       glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},   // Top-Right

            // Back face (-Z)
            {glm::vec3(0.5f, 0.5f, -0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},  // Top-Left
            {glm::vec3(0.5f, -0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)},  // Bottom-Left
            {glm::vec3(-0.5f, -0.5f, -0.5f),    glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)},  // Bottom-Right
            {glm::vec3(-0.5f, 0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},  // Top-Right

            // Left face (-X)
            {glm::vec3(-0.5f, 0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},  // Top-Left
            {glm::vec3(-0.5f, -0.5f, -0.5f),    glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},  // Bottom-Left
            {glm::vec3(-0.5f, -0.5f, 0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},  // Bottom-Right
            {glm::vec3(-0.5f, 0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},  // Top-Right

            // Right face (+X)
            {glm::vec3(0.5f, 0.5f, 0.5f),       glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},   // Top-Left
            {glm::vec3(0.5f, -0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},   // Bottom-Left
            {glm::vec3(0.5f, -0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},   // Bottom-Right
            {glm::vec3(0.5f, 0.5f, -0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},   // Top-Right

            // Top face (+Y)
            {glm::vec3(-0.5f, 0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},   // Top-Left
            {glm::vec3(-0.5f, 0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},   // Bottom-Left
            {glm::vec3(0.5f, 0.5f, 0.5f),       glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},   // Bottom-Right
            {glm::vec3(0.5f, 0.5f, -0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},   // Top-Right

            // Bottom face (-Y)
            {glm::vec3(-0.5f, -0.5f, 0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},  // Top-Left
            {glm::vec3(-0.5f, -0.5f, -0.5f),    glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},  // Bottom-Left
            {glm::vec3(0.5f, -0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},  // Bottom-Right
            {glm::vec3(0.5f, -0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},  // Top-Right
        };
        unsigned numVertices = sizeof(vertices) / sizeof(Vertex);

        GLuint indices[] =
        {
            0,  1,  2,  0,  2,  3,  // Front face
            4,  5,  6,  4,  6,  7,  // Back face
            8,  9,  10, 8,  10, 11, // Left face
            12, 13, 14, 12, 14, 15, // Right face
            16, 17, 18, 16, 18, 19, // Top face
            20, 21, 22, 20, 22, 23  // Bottom face
        };
        unsigned numIndices = sizeof(indices) / sizeof(GLuint);

        set(vertices, numVertices, indices, numIndices);
    }
};

class Custom : public Primitive
{
public:
    Custom(Vertex *vertices, unsigned int numVertices,
           GLuint *indices, unsigned int numIndices) : Primitive()
    {
        set(vertices, numVertices, indices, numIndices);
    }
};
