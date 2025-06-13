#pragma once

#include "Shader.hpp"

// Represents the material properties of an object.
class Material
{
private:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

public:
    // Constructor
    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
        : ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}

    // Destructor
    ~Material() {}

    // Sends the material properties to the shader.
    void sendToShader(Shader &program)
    {
        program.setVec3("material.ambient", ambient);
        program.setVec3("material.diffuse", diffuse);
        program.setVec3("material.specular", specular);
        program.setFloat("material.shininess", shininess);
    }
};
