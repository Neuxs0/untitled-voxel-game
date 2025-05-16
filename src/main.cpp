#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void glfw_error_callback(int error, const char *description) {
    std::cerr << "GLFW Error (Code " << error << "): " << description << std::endl;
}

GLFWwindow* createWindow(int width, int height, const char* title) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // No Resizing
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (window == NULL) {
        std::cerr << "Error:main.cpp:createWindow: GLFW Window failed to create" << "\n";
        return nullptr;
    }

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();

    if (glewErr != GLEW_OK) {
        std::cerr << "Error:main.cpp: createWindow: GLEW Initialization Failed: " << glewGetErrorString(glewErr) << "\n";
        glfwDestroyWindow(window);
        return nullptr;
    }

    // Show OpenGL Details
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return window;
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Error:main.cpp: glfwInit() failed." << std::endl;
        return -1;
    }

    const int winWidth = 640;
    const int winHeight = 480;

    const char* winTitle = "Window Title :D";

    GLFWwindow* window = createWindow(winWidth, winHeight, winTitle);

    if (!window) {
        std::cerr << "Error:main.cpp: Failed to create window or initialize graphics context" << "\n";
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        glfwSwapBuffers(window);
        glFlush();
    }


    glfwTerminate();
    return 0;
}
