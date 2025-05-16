#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texcoord;
};


std::vector<Vertex> vertices = {
    // position                     // color                        // texcoord
    {glm::vec3(0.0f, 0.5f, 0.0f),   glm::vec3(0.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 1.0f)},
    {glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(0.0f, 0.0f)},
    {glm::vec3(0.5f, -0.5f, 0.0f),  glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec2(1.0f, 0.0f)}
};

std::vector<GLuint> indices = {
    0, 1, 2
};

void updateInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

std::string readFileToString(const std::string &path, bool &success) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR:main.cpp:readFileToString: Failed to open file: " << path << std::endl;
        success = false;
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (file.bad()) {
        std::cerr << "ERROR:main.cpp:readFileToString: Failed to read file: " << path << std::endl;
        success = false;
        return "";
    }

    file.close();
    success = true;
    return content;
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

GLFWwindow* createWindow(const int width, const int height, const char* title) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

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
        glfwTerminate();
        return -1;
    }


    // OpenGL Options
    glfwSwapInterval(1); // 1 for VSync on, 0 for VSync off

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Initialize Shaders
    GLuint core_program;
    if (!loadShaders(core_program, "shaders/core.vert.glsl", "shaders/core.frag.glsl")) {
        glfwTerminate();
        return -1;
    }


    // Create and bind VAO, VBO, and EBO
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Input Assembly
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    
    glUseProgram(core_program);
    glBindVertexArray(VAO);


    // App Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        updateInput(window);

        // Clear Screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Draw
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

        // End Draw
        glfwSwapBuffers(window);
    }

    glUseProgram(0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    glDeleteProgram(core_program);
    return 0;
}
