#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Namespace for all window-related functions and variables.
namespace Window
{
    // Camera vectors
    extern glm::vec3 cameraFront;
    extern glm::vec3 cameraUp;
    extern glm::vec3 cameraRight;
    // Camera rotation
    extern float yaw;
    extern float pitch;
    // Mouse position
    extern double lastX;
    extern double lastY;
    extern bool firstMouse;
    // Wireframe mode toggle
    extern bool wireframeEnabled;

    // GLFW error callback function.
    void glfw_error_callback(int error, const char *description);
    // GLFW framebuffer resize callback function.
    void framebuffer_resize_callback(GLFWwindow *window, int fbWidth, int fbHeight);
    // GLFW mouse movement callback function.
    void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    // Initialize GLFW, GLEW, and create a window.
    GLFWwindow *initialize(const int width, const int height, const char *title, bool &success);
    // Process input from the user.
    void updateInput(GLFWwindow *window, float deltaTime, glm::vec3 &camPos);
}
