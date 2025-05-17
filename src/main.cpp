#include <vector>

#include "window.h"
#include "Vertex.h"
#include "Shader.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>


std::vector<Vertex> vertices =
{
    // position                         // color                        // texcoord             // normal
    // Front face (+Z)
    {glm::vec3(-0.5f, 0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)},   // Top-Left
    {glm::vec3(-0.5f, -0.5f, 0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f)},   // Bottom-Left
    {glm::vec3(0.5f, -0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 1.0f)},   // Bottom-Right
    {glm::vec3(0.5f, 0.5f, 0.5f),       glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)},   // Top-Right

    // Back face (-Z)
    {glm::vec3(0.5f, 0.5f, -0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f),  glm::vec3(0.0f, 0.0f, -1.0f)},  // Top-Left
    {glm::vec3(0.5f, -0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, -1.0f)},  // Bottom-Left
    {glm::vec3(-0.5f, -0.5f, -0.5f),    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f),  glm::vec3(0.0f, 0.0f, -1.0f)},  // Bottom-Right
    {glm::vec3(-0.5f, 0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, -1.0f)},  // Top-Right

    // Left face (-X)
    {glm::vec3(-0.5f, 0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f),  glm::vec3(-1.0f, 0.0f, 0.0f)},  // Top-Left
    {glm::vec3(-0.5f, -0.5f, -0.5f),    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f),  glm::vec3(-1.0f, 0.0f, 0.0f)},  // Bottom-Left
    {glm::vec3(-0.5f, -0.5f, 0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f),  glm::vec3(-1.0f, 0.0f, 0.0f)},  // Bottom-Right
    {glm::vec3(-0.5f, 0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f),  glm::vec3(-1.0f, 0.0f, 0.0f)},  // Top-Right

    // Right face (+X)
    {glm::vec3(0.5f, 0.5f, 0.5f),       glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f)},   // Top-Left
    {glm::vec3(0.5f, -0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f)},   // Bottom-Left
    {glm::vec3(0.5f, -0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f)},   // Bottom-Right
    {glm::vec3(0.5f, 0.5f, -0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f),  glm::vec3(1.0f, 0.0f, 0.0f)},   // Top-Right

    // Top face (+Y)
    {glm::vec3(-0.5f, 0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)},   // Top-Left
    {glm::vec3(-0.5f, 0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f)},   // Bottom-Left
    {glm::vec3(0.5f, 0.5f, 0.5f),       glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f)},   // Bottom-Right
    {glm::vec3(0.5f, 0.5f, -0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)},   // Top-Right

    // Bottom face (-Y)
    {glm::vec3(-0.5f, -0.5f, 0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f),  glm::vec3(0.0f, -1.0f, 0.0f)},  // Top-Left
    {glm::vec3(-0.5f, -0.5f, -0.5f),    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)},  // Bottom-Left
    {glm::vec3(0.5f, -0.5f, -0.5f),     glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)},  // Bottom-Right
    {glm::vec3(0.5f, -0.5f, 0.5f),      glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f),  glm::vec3(0.0f, -1.0f, 0.0f)},  // Top-Right
};

std::vector<GLuint> indices =
{
    0,  1,  2,  0,  2,  3,  // Front face
    4,  5,  6,  4,  6,  7,  // Back face
    8,  9,  10, 8,  10, 11, // Left face
    12, 13, 14, 12, 14, 15, // Right face
    16, 17, 18, 16, 18, 19, // Top face
    20, 21, 22, 20, 22, 23  // Bottom face
};


int main()
{
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Window Settings
    const int winWidth = 800;
    const int winHeight = 600;
    const char* winTitle = "Untitled Voxel Game - Camera Demo";

    bool windowInitSuccess = false;
    GLFWwindow* window = Window::initialize(winWidth, winHeight, winTitle, windowInitSuccess);

    if (!windowInitSuccess || !window)
        return -1;
    
    // Show OpenGL Details
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    // OpenGL Options
    glfwSwapInterval(0); // 0 for VSync off, 1 for VSync on
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize Shaders
    Shader coreShader("core.vert.glsl", "core.frag.glsl");

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);

    // Initialize Textures
    Texture dirtTex("dirt.png", 0);

    // Model Transformation
    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation(0.0f);
    glm::vec3 scale(1.0f);

    glm::mat4 ModelMatrix(1.0f);
    ModelMatrix = glm::translate(ModelMatrix, position);
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    ModelMatrix = glm::scale(ModelMatrix, scale);

    // Camera
    glm::vec3 camPos(0.0f, 1.0f, 2.0f);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    glm::mat4 ViewMatrix(1.0f);
    ViewMatrix = glm::lookAt(camPos, camPos + Window::cameraFront, worldUp);

    float fov = 85.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    glm::mat4 ProjectionMatrix(1.0f);
    ProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(fbWidth) / fbHeight, nearPlane, farPlane);

    // Lighting
    glm::vec3 lightPos0(1.0f, 2.0f, 1.0f);

    // Initialize uniforms
    coreShader.use();
    coreShader.setInt("texture0", dirtTex.getTexUnit());
    coreShader.setVec3("lightPos0", lightPos0);
    coreShader.setVec3("cameraPos", camPos);
    coreShader.setMat4("ModelMatrix", ModelMatrix);
    coreShader.setMat4("ViewMatrix", ViewMatrix);
    coreShader.setMat4("ProjectionMatrix", ProjectionMatrix);
    coreShader.unuse();

    // App Loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // DeltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Window::updateInput(window, deltaTime, camPos);

        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // Translations
        glm::mat4 ModelMatrix(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, position);
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scale));

        ProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(fbWidth) / fbHeight, nearPlane, farPlane);

        ViewMatrix = glm::lookAt(camPos, camPos + Window::cameraFront, worldUp);

        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        coreShader.use();
        glBindVertexArray(VAO);

        // Update uniforms
        coreShader.setBool("texture0", dirtTex.getTexUnit());
        coreShader.setVec3("cameraPos", camPos);
        coreShader.setVec3("lightPos0", lightPos0);
        coreShader.setMat4("ViewMatrix", ViewMatrix);
        coreShader.setMat4("ProjectionMatrix", ProjectionMatrix);

        // Activate textures
        dirtTex.bind();

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
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
