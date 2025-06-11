#include <iostream>
#include <GL/glew.h>
#include "Window.hpp"
#include "Camera.hpp"

namespace Window
{
    // Mouse position
    double lastX = 0.0;
    double lastY = 0.0;
    bool firstMouse = true;

    // Wireframe mode toggle
    bool wireframeEnabled = false;

    // GLFW error callback function.
    void glfw_error_callback(int error, const char *description)
    {
        std::cerr << "GLFW Error (Code " << error << "): " << description << std::endl;
    }

    // GLFW framebuffer resize callback function.
    void framebuffer_resize_callback(GLFWwindow *window, int fbWidth, int fbHeight)
    {
        (void)window; // Unused parameter
        glViewport(0, 0, fbWidth, fbHeight);
    }

    // GLFW mouse movement callback function.
    void mouse_callback(GLFWwindow *window, double xpos, double ypos)
    {
        Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
        if (!camera)
            return;

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(lastY - ypos);
        lastX = xpos;
        lastY = ypos;

        camera->processMouseMovement(xoffset, yoffset);
    }

    // Process input from the user.
    void updateInput(GLFWwindow *window, Camera &camera, float deltaTime)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.processKeyboard(CameraMovement::DOWN, deltaTime);

        // Toggle wireframe mode
        bool key0_is_pressed = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;
        bool key0_pressed_last_frame = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;
        if (key0_is_pressed && !key0_pressed_last_frame)
        {
            wireframeEnabled = !wireframeEnabled;
            if (wireframeEnabled)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        key0_pressed_last_frame = key0_is_pressed;
    }

    // Initialize GLFW, GLEW, and create a window.
    GLFWwindow *initialize(const int width, const int height, const char *title, bool &initSuccess)
    {
        glfwSetErrorCallback(Window::glfw_error_callback);

        if (!glfwInit())
        {
            std::cerr << "Error: Window: initialize: glfwInit() failed." << std::endl;
            initSuccess = false;
            return nullptr;
        }

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        glfwWindowHintString(GLFW_X11_CLASS_NAME, "untitled_voxel_game");
        glfwWindowHintString(GLFW_WAYLAND_APP_ID, "untitled_voxel_game");

        GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (window == NULL)
        {
            std::cerr << "Error: Window: initialize: GLFW Window failed to create." << std::endl;
            glfwTerminate();
            initSuccess = false;
            return nullptr;
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, Window::framebuffer_resize_callback);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        lastX = static_cast<double>(width) / 2.0;
        lastY = static_cast<double>(height) / 2.0;
        glfwSetCursorPosCallback(window, Window::mouse_callback);

        glewExperimental = GL_TRUE;
        GLenum glewErr = glewInit();
        if (glewErr != GLEW_OK)
        {
            std::cerr << "Error: Window: initialize: GLEW Initialization Failed: " << glewGetErrorString(glewErr) << std::endl;
            glfwDestroyWindow(window);
            glfwTerminate();
            initSuccess = false;
            return nullptr;
        }

        initSuccess = true;
        return window;
    }

} // namespace Window
