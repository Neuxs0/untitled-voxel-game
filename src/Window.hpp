#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Window
{
    extern glm::vec3 cameraFront;
    extern glm::vec3 cameraUp;
    extern glm::vec3 cameraRight;
    extern float yaw;
    extern float pitch;
    extern double lastX;
    extern double lastY;
    extern bool firstMouse;
    extern bool wireframeEnabled;

    void glfw_error_callback(int error, const char *description);
    void framebuffer_resize_callback(GLFWwindow *window, int fbWidth, int fbHeight);
    void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    GLFWwindow *initialize(const int width, const int height, const char *title, bool &success);
    void updateInput(GLFWwindow *window, float deltaTime, glm::vec3 &camPos);
}
