#include "window.h"
#include <iostream>
#include <GL/glew.h>

namespace Window {

void glfw_error_callback(int error, const char *description) {
    std::cerr << "GLFW Error (Code " << error << "): " << description << std::endl;
}

void framebuffer_resize_callback(GLFWwindow* window, int fbWidth, int fbHeight) {
    (void)window; // To prevent unused parameter warning
    glViewport(0, 0, fbWidth, fbHeight);
}

void updateInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWwindow* initialize(const int width, const int height, const char* title, bool &initSuccess) {
    glfwSetErrorCallback(Window::glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Error:Window:initialize: glfwInit() failed." << std::endl;
        initSuccess = false;
        return nullptr;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // GLFW 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS if needed

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) {
        std::cerr << "Error:Window:initialize: GLFW Window failed to create." << std::endl;
        glfwTerminate();
        initSuccess = false;
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, Window::framebuffer_resize_callback);
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "Error:Window:initialize: GLEW Initialization Failed: " << glewGetErrorString(glewErr) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        initSuccess = false;
        return nullptr;
    }
    
    initSuccess = true;
    return window;
}

} // namespace Window
