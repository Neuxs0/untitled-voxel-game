#include <iostream>
#include <vector>

#include "window.h"
#include "rendering/Vertex.h"
#include "shader/shader_loader.h"
#include "texture/texture_loader.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<Vertex> vertices = {
    // position                     // color                        // texcoord
    {glm::vec3(-0.5f, 0.5f, 0.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f)},
    {glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f)},
    {glm::vec3(0.5f, -0.5f, 0.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f)},
    {glm::vec3(0.5f, 0.5f, 0.0f),   glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f)}
};

std::vector<GLuint> indices = {
    0, 1, 2,    // Triangle 1
    0, 2, 3     // Triangle 2
};


int main() {
    // Window Settings
    const int winWidth = 640;
    const int winHeight = 480;
    const char* winTitle = "Untitled Voxel Game";

    bool windowInitSuccess = false;
    GLFWwindow* window = Window::initialize(winWidth, winHeight, winTitle, windowInitSuccess);

    if (!windowInitSuccess || !window) {
        return -1;
    }
    
    // Show OpenGL Details
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    int fbWidth, fbHeight;

    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    // OpenGL Options
    glfwSwapInterval(1); // 1 for VSync on, 0 for VSync off

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Shaders
    GLuint core_program;
    if (!ShaderLoader::loadShadersFromFile(core_program, "assets/shaders/core.vert.glsl", "assets/shaders/core.frag.glsl")) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }


    // Create and bind VAO, VBO, and EBO
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);


    // Vertex Attribute Pointers
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);


    // Initialize Textures
    GLuint texture0ID;
    int texWidth = 0;
    int texHeight = 0;
    if (!TextureLoader::loadTexture("assets/textures/dirt.png", texture0ID, texWidth, texHeight)) {
        std::cerr << "Error: main: Failed to initialize texture." << std::endl;
        return -1;
    }


    // Translations
    glm::mat4 ModelMatrix(1.0f);
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));


    // View projection
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 cameFront(0.0f, 0.0f, -1.0f);
    glm::vec3 camPosition(0.0f, 0.0f, 2.0f);

    glm::mat4 ViewMatrix(1.0f);
    ViewMatrix = glm::lookAt(camPosition, camPosition + cameFront, worldUp);

    float fov = 85.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    glm::mat4 ProjectionMatrix(1.0f);
    ProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(fbWidth) / fbHeight, nearPlane, farPlane);


    // Initialize uniforms
    glUseProgram(core_program);

    glUniformMatrix4fv(glGetUniformLocation(core_program, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(core_program, "ViewMatrix"), 1, GL_FALSE, glm::value_ptr(ViewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(core_program, "ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));

    glUseProgram(0);


    // App Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Window::updateInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glUseProgram(core_program);
        glBindVertexArray(VAO);

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // Translations
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));
        ProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(fbWidth) / fbHeight, nearPlane, farPlane);

        // Update uniforms
        glUniform1i(glGetUniformLocation(core_program, "texture0"), 0);
        glUniformMatrix4fv(glGetUniformLocation(core_program, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(core_program, "ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));

        // Activate textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0ID);

        // Draw
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

        // End draw
        glfwSwapBuffers(window);

        glBindVertexArray(0);
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    

    // Cleanup
    glDeleteProgram(core_program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture0ID);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
