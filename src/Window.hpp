#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Forward declaration
class Camera;

// A class to manage the GLFW window, its state, and input handling.
class Window
{
private:
    GLFWwindow *m_window;
    int m_width;
    int m_height;
    const char *m_title;

    // Mouse state
    double m_lastX;
    double m_lastY;
    bool m_firstMouse;

    // Input state
    bool m_wireframeEnabled;

    // Pointer to the main camera for input processing
    Camera *m_camera;

    // GLFW static callback functions
    static void glfw_error_callback(int error, const char *description);
    static void framebuffer_resize_callback(GLFWwindow *window, int fbWidth, int fbHeight);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

public:
    // Constructor and Destructor
    Window(int width, int height, const char *title);
    ~Window();

    // Non-copyable and non-movable
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    // Initializes GLFW, GLEW, and creates the window.
    bool initialize();

    // Process user input for the current frame.
    void updateInput(float deltaTime);

    // Check if the window should be closed.
    bool shouldClose() const;

    // Swap front and back buffers and poll for events.
    void swapBuffersAndPollEvents();

    // Set the camera for input handling.
    void setCamera(Camera *camera);

    // Getters
    bool isWireframeEnabled() const;
};
