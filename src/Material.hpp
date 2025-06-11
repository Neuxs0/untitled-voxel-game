#pragma once

#include "Shader.hpp"

class Material
{
private:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

public:
    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
        : ambient(ambient), diffuse(diffuse), specular(specular)
    {
        
    }

    ~Material() {}


    void sendToShader(Shader &program)
    {
        program.setVec3("material.ambient", ambient);
        program.setVec3("material.diffuse", diffuse);
        program.setVec3("material.specular", specular);
    }
};
