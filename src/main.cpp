#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string readFileToString(const std::string &path, bool &success) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "ERROR:main.cpp:readFileToString: Failed to open file (mode: binary | ate): " << path << std::endl;
        success = false;
        return "";
    }

    std::streamsize size = file.tellg();

    if (size == static_cast<std::streamsize>(-1)) {
        std::cerr << "ERROR:main.cpp:readFileToString: Failed to determine file size for: " << path << std::endl;
        file.close();
        success = false;
        return "";
    }

    if (size == 0) {
        file.close();
        success = true;
        return "";
    }

    file.seekg(0, std::ios::beg);

    if (!file.good()) {
        std::cerr << "ERROR:main.cpp:readFileToString: Failed to seek to beginning of file: " << path << std::endl;
        file.close();
        success = false;
        return "";
    }

    std::string buffer;
    buffer.resize(static_cast<size_t>(size));

    if (!file.read(&buffer[0], size)) {
        std::cerr << "ERROR:main.cpp:readFileToString: Failed to read " << size << " bytes from file: " << path << std::endl;
        file.close();
        success = false;
        return "";
    }

    file.close();
    success = true;
    return buffer;
}

void glfw_error_callback(int error, const char *description) {
    std::cerr << "GLFW Error (Code " << error << "): " << description << std::endl;
}

void framebuffer_resize_callback(GLFWwindow* window, int fbWidth, int fbHeight) {
    glViewport(0, 0, fbWidth, fbHeight);
}

bool loadShaders(GLuint &program, const std::string &vertShaderPath, const std::string &fragShaderPath) {
    char infoLog[512];
    GLint success;
    bool fileReadSuccess = true;

    std::string src;

    // Vertex
    src = readFileToString(vertShaderPath, fileReadSuccess);
    if (!fileReadSuccess) {
        std::cerr << "Error:main.cpp:loadShaders: Failed to read vertex shader file: " << vertShaderPath << "\n";
        return false;
    }

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertSrc = src.c_str();
    glShaderSource(vertShader, 1, &vertSrc, NULL);
    glCompileShader(vertShader);

    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        std::cerr << "Error:main.cpp:loadShaders: Couldn't compile vertex shader: " << vertShaderPath << "\n";
        std::cerr << infoLog << "\n";
        return false;
    }

    // Fragment
    src = readFileToString(fragShaderPath, fileReadSuccess);
    if (!fileReadSuccess) {
        std::cerr << "Error:main.cpp:loadShaders: Failed to read fragment shader file: " << fragShaderPath << "\n";
        return false;
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragSrc = src.c_str();
    glShaderSource(fragShader, 1, &fragSrc, NULL);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cerr << "Error:main.cpp:loadShaders: Couldn't compile fragment shader: " << fragShaderPath << "\n";
        std::cerr << infoLog << "\n";
        return false;
    }

    // Program
    program = glCreateProgram();

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Error:main.cpp:loadShaders: Couldn't link program" << "\n";
        std::cerr << infoLog << "\n";
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    // End
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return true;
}

GLFWwindow* createWindow(int width, int height, const char* title) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW 4.4
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    // Make window resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (window == NULL) {
        std::cerr << "Error:main.cpp:createWindow: GLFW Window failed to create" << "\n";
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    glfwMakeContextCurrent(window);

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

    // Window Settings
    const int winWidth = 640;
    const int winHeight = 480;

    const char* winTitle = "Untitled Voxel Game";

    // Initialize Window
    GLFWwindow* window = createWindow(winWidth, winHeight, winTitle);

    if (!window) {
        std::cerr << "Error:main.cpp: Failed to create window or initialize graphics context" << "\n";
        glfwTerminate();
        return -1;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();

    if (glewErr != GLEW_OK) {
        std::cerr << "Error:main.cpp:createWindow: GLEW Initialization Failed: " << glewGetErrorString(glewErr) << "\n";
        glfwDestroyWindow(window);
        return -1;
    }

    glfwSwapInterval(1); // 1 for VSync on, 0 for VSync off

    // Initialize Shaders
    GLuint core_program;
    if (!loadShaders(core_program, "shaders/core.vert.glsl", "shaders/core.frag.glsl")) {
        glfwTerminate();
        return -1;
    }

    // App Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear Screen
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Draw
        glUseProgram(core_program);

        // End Draw
        glUseProgram(0);
        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);
    glfwTerminate();
    glDeleteProgram(core_program);
    return 0;
}
