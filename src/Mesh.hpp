#pragma once

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "Primitives.hpp"
#include "Shader.hpp"
#include "Material.hpp"

class Mesh {
private:
    unsigned numVertices;
    unsigned numIndices;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::mat4 ModelMatrix;

    void initVAO(Primitive *primitive)
    {
        this->numVertices = primitive->getNumVertices();
        this->numIndices = primitive->getNumIndices();

        // Bind VAO, VBO, and EBO
        glCreateVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), primitive->getVertices(), GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), primitive->getIndices(), GL_STATIC_DRAW);

        // Vertex Attribute Pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void updateUniforms(Shader *shader)
    {
        shader->setMat4("ModelMatrix", ModelMatrix);
    }

    void updateModelMatrix()
    {
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, position);
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        ModelMatrix = glm::scale(ModelMatrix, scale);
    }


public:
    Mesh(Primitive *primitive,
         glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        initVAO(primitive);
        updateModelMatrix();
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }


    // inline void setPos(const glm::vec3 &pos) { position = pos; }
    // inline void setRot(const glm::vec3 &rot) { rotation = rot; }
    // inline void setScale(const glm::vec3 &scale) { this->scale = scale; }

    // inline void move(const glm::vec3 &pos) { position += pos; }
    // inline void rotate(const glm::vec3 &rot) { rotation += rot; }
    // inline void scale(const glm::vec3 &scale) { this->scale += scale; }


    void update()
    {
    }

    void render(Shader *shader)
    {
        updateModelMatrix();
        updateUniforms(shader);

        shader->use();

        // Bind VAO
        glBindVertexArray(VAO);

        // Render
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    }
};
