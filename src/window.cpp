#include <iostream>
#include <GL/glew.h>

#include "window.h"

namespace Window
{

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
float yaw = -90.0f;
float pitch = 0.0f;
double lastX = 0.0;
double lastY = 0.0;
bool firstMouse = true;

void glfw_error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error (Code " << error << "): " << description << std::endl;
}

void framebuffer_resize_callback(GLFWwindow* window, int fbWidth, int fbHeight)
{
    (void)window; // To prevent unused parameter warning
    glViewport(0, 0, fbWidth, fbHeight);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void)window; // To prevent unused parameter warning
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

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void updateInput(GLFWwindow *window, float deltaTime, glm::vec3 &camPos)
{
    const float camSpeed = 1.7f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 localCameraRight = glm::normalize(glm::cross(cameraFront, worldUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += cameraFront * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= cameraFront * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= localCameraRight * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += localCameraRight * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos += worldUp * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camPos -= worldUp * camSpeed;
}

GLFWwindow* initialize(const int width, const int height, const char *title, bool &initSuccess)
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
