#pragma once

#include "Shader.hpp"

class Material
{
private:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    GLint diffuseTex;
    GLint specularTex;

public:
    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLint diffuseTex, GLint specularTex)
        : ambient(ambient), diffuse(diffuse), specular(specular), diffuseTex(diffuseTex), specularTex(specularTex)
    {
        
    }

    ~Material() {}


    void sendToShader(Shader &program)
    {
        program.setVec3("material.ambient", ambient);
        program.setVec3("material.diffuse", diffuse);
        program.setVec3("material.specular", specular);
        program.setInt("material.diffuseTex", diffuseTex);
        program.setInt("material.specularTex", specularTex);
    }
};
