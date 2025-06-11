#include "window.hpp"
#include "Mesh.hpp"

int main()
{
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    int frameCount = 0;
    double lastFrameTime = glfwGetTime();

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

    // Initialize Mesh
    Cube dirtBlock;
    Mesh mesh(&dirtBlock);

    // Initialize Materials
    //                    Ambient          Diffuse          Specular
    Material dirtMaterial(glm::vec3(0.2f), glm::vec3(1.0f), glm::vec3(0.1f));


    // Model Transformation
    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation(0.0f);
    glm::vec3 scale(1.0f);

    // Camera
    glm::vec3 camPos(0.0f, 1.0f, 2.0f);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    glm::mat4 ViewMatrix(1.0f);
    ViewMatrix = glm::lookAt(camPos, camPos + Window::cameraFront, worldUp);

    float fov = 85.0f;
    float nearPlane = 0.01f;
    float farPlane = 1000.0f;
    glm::mat4 ProjectionMatrix(1.0f);
    ProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(fbWidth) / fbHeight, nearPlane, farPlane);

    // Lighting
    glm::vec3 lightPos0(1.0f, 2.0f, 1.0f);

    // Initialize uniforms
    coreShader.use();
    coreShader.setVec3("lightPos0", lightPos0);
    coreShader.setVec3("cameraPos", camPos);
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

        frameCount++;
        if (currentFrame - lastFrameTime >= 1.0)
        {
            double actual_interval = currentFrame - lastFrameTime;
            int fps = frameCount / actual_interval;
            std::cout << "FPS: " << fps << std::endl;

            frameCount = 0;
            lastFrameTime = currentFrame;
        }

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

        // Update uniforms
        coreShader.setVec3("cameraPos", camPos);
        coreShader.setVec3("lightPos0", lightPos0);
        coreShader.setMat4("ViewMatrix", ViewMatrix);
        coreShader.setMat4("ProjectionMatrix", ProjectionMatrix);
        dirtMaterial.sendToShader(coreShader);

        mesh.render(&coreShader);

        // End draw
        glfwSwapBuffers(window);

        glBindVertexArray(0);
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
