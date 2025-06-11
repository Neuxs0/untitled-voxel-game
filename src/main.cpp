#include "Window.hpp"
#include "Shader.hpp"
#include "Material.hpp"
#include "World.hpp"
#include <glm/gtc/matrix_transform.hpp>

int main()
{
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    int frameCount = 0;
    double lastFrameTime = glfwGetTime();

    // Window Settings
    const int winWidth = 800;
    const int winHeight = 600;
    const char* winTitle = "Untitled Voxel Game";

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

    // Initialize the World. This creates and meshes our first chunk.
    World world;

    // A single material to be used for all blocks for now
    Material blockMaterial(glm::vec3(0.2f), glm::vec3(1.0f), glm::vec3(0.05f));

    // Camera
    glm::vec3 camPos(0.0f, 1.8f, 0.0f);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::mat4 ViewMatrix(1.0f);
    
    // Projection
    float fov = 85.0f;
    float nearPlane = 0.01f;
    float farPlane = 1000.0f;
    glm::mat4 ProjectionMatrix(1.0f);

    // Lighting
    glm::vec3 lightPos0(32.0f, 32.0f, 32.0f);

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

        // Update matrices
        ProjectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(fbWidth) / fbHeight, nearPlane, farPlane);
        ViewMatrix = glm::lookAt(camPos, camPos + Window::cameraFront, worldUp);

        // Clear screen with a sky-blue color
        glClearColor(0.1f, 0.4f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Prepare shader and set uniforms
        coreShader.use();
        coreShader.setVec3("cameraPos", camPos);
        coreShader.setVec3("lightPos0", lightPos0);
        coreShader.setMat4("ViewMatrix", ViewMatrix);
        coreShader.setMat4("ProjectionMatrix", ProjectionMatrix);
        blockMaterial.sendToShader(coreShader);

        coreShader.setBool("u_isWireframe", Window::wireframeEnabled);

        // Render the world
        world.render(coreShader);

        // End draw
        glfwSwapBuffers(window);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
