#include "Window.hpp"
#include "Shader.hpp"
#include "Material.hpp"
#include "World.hpp"
#include "Camera.hpp"
#include "EmbeddedShaders.hpp"
#include "Constants.hpp" // Required for Constants::TEXTURE_SIZE_PX
#include <glm/gtc/matrix_transform.hpp>

int main()
{
    // Frame timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    int frameCount = 0;
    double lastFrameTime = glfwGetTime();

    // Window properties
    const int winWidth = 800;
    const int winHeight = 600;
    const char *winTitle = "Untitled Voxel Game";

    // Initialize window
    Window window(winWidth, winHeight, winTitle);
    if (!window.initialize())
        return -1;

    // Camera setup
    Camera camera(glm::vec3(0.8f, 10.8f, 0.8f), winWidth, winHeight);
    window.setCamera(&camera);

    // OpenGL information
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shader setup using the embedded sources
    Shader coreShader(EmbeddedShaders::core_vert, EmbeddedShaders::core_frag);

    // World setup
    World world; 

    // Set uniforms now that the atlas is ready
    coreShader.use(); 
    glm::vec2 tileSize = world.getAtlasNormalizedTileSize();
    if (tileSize.x > 0.0f && tileSize.y > 0.0f) { 
         coreShader.setVec2("u_atlasTileSize", tileSize);
    } else {
        std::cerr << "Warning: Atlas tile size is zero in main. Textures might not render correctly." << std::endl;
    }
    coreShader.setFloat("u_texturePixelDimension", static_cast<float>(Constants::TEXTURE_SIZE_PX));


    // Material setup
    Material blockMaterial(glm::vec3(0.2f), glm::vec3(1.0f), glm::vec3(0.05f), 32.0f);

    // Light setup
    glm::vec3 lightPos0(8.0f, 30.0f, 8.0f);

    // Main game loop
    while (!window.shouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frameCount++;
        if (currentFrame - lastFrameTime >= 1.0)
        {
            double actual_interval = currentFrame - lastFrameTime;
            int fps = static_cast<int>(frameCount / actual_interval);
            std::cout << "FPS: " << fps << std::endl;
            frameCount = 0;
            lastFrameTime = currentFrame;
        }

        window.updateInput(deltaTime);
        world.update(camera.getPosition());

        glm::mat4 ViewMatrix = camera.getViewMatrix();
        camera.updateFrustum(ViewMatrix);

        glClearColor(0.1f, 0.4f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        coreShader.use();
        coreShader.setVec3("cameraPos", camera.getPosition());
        coreShader.setVec3("lightPos0", lightPos0);
        coreShader.setMat4("ViewMatrix", ViewMatrix);
        coreShader.setMat4("ProjectionMatrix", camera.getProjectionMatrix());
        blockMaterial.sendToShader(coreShader);
        coreShader.setBool("u_isWireframe", window.isWireframeEnabled());
        world.render(coreShader, camera);

        window.swapBuffersAndPollEvents();
    }

    return 0;
}
