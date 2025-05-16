#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Window {
    void glfw_error_callback(int error, const char *description);
    void framebuffer_resize_callback(GLFWwindow* window, int fbWidth, int fbHeight);
    GLFWwindow* initialize(const int width, const int height, const char* title, bool &success);
    void updateInput(GLFWwindow* window);
}
