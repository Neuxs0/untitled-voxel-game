#include <iostream>
#include <GL/glew.h>
#include "Window.hpp"
#include "Camera.hpp"

// Constructor
Window::Window(int width, int height, const char *title)
    : m_window(nullptr), m_width(width), m_height(height), m_title(title),
      m_lastX(0.0), m_lastY(0.0), m_firstMouse(true), m_wireframeEnabled(false), m_camera(nullptr)
{
}

// Destructor
Window::~Window()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

// GLFW error callback function (static).
void Window::glfw_error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error (Code " << error << "): " << description << std::endl;
}

// GLFW framebuffer resize callback function (static).
void Window::framebuffer_resize_callback(GLFWwindow *window, int fbWidth, int fbHeight)
{
    glViewport(0, 0, fbWidth, fbHeight);

    // Get the window instance and update the camera projection matrix.
    Window *winInstance = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (winInstance && winInstance->m_camera)
    {
        winInstance->m_camera->updateProjectionMatrix(fbWidth, fbHeight);
    }
}

// GLFW mouse movement callback function (static).
void Window::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    Window *winInstance = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (!winInstance || !winInstance->m_camera)
        return;

    if (winInstance->m_firstMouse)
    {
        winInstance->m_lastX = xpos;
        winInstance->m_lastY = ypos;
        winInstance->m_firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - winInstance->m_lastX);
    float yoffset = static_cast<float>(winInstance->m_lastY - ypos);
    winInstance->m_lastX = xpos;
    winInstance->m_lastY = ypos;

    winInstance->m_camera->processMouseMovement(xoffset, yoffset);
}

// Process input from the user.
void Window::updateInput(float deltaTime)
{
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);

    if (m_camera)
    {
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
            m_camera->processKeyboard(CameraMovement::FORWARD, deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
            m_camera->processKeyboard(CameraMovement::BACKWARD, deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
            m_camera->processKeyboard(CameraMovement::LEFT, deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
            m_camera->processKeyboard(CameraMovement::RIGHT, deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_camera->processKeyboard(CameraMovement::UP, deltaTime);
        if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            m_camera->processKeyboard(CameraMovement::DOWN, deltaTime);
    }

    // Toggle wireframe mode
    static bool key0_pressed_last_frame = false;
    bool key0_is_pressed = glfwGetKey(m_window, GLFW_KEY_0) == GLFW_PRESS;

    if (key0_is_pressed && !key0_pressed_last_frame)
    {
        m_wireframeEnabled = !m_wireframeEnabled;
        if (m_wireframeEnabled)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    key0_pressed_last_frame = key0_is_pressed;
}

// Initialize GLFW, GLEW, and create a window.
bool Window::initialize()
{
    glfwSetErrorCallback(Window::glfw_error_callback);

    if (!glfwInit())
    {
        std::cerr << "Error: Window: initialize: glfwInit() failed." << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "untitled_voxel_game");
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "untitled_voxel_game");

    m_window = glfwCreateWindow(m_width, m_height, m_title, NULL, NULL);
    if (m_window == NULL)
    {
        std::cerr << "Error: Window: initialize: GLFW Window failed to create." << std::endl;
        glfwTerminate();
        return false;
    }

    // Store 'this' pointer to retrieve in static callbacks
    glfwSetWindowUserPointer(m_window, this);

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, Window::framebuffer_resize_callback);

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_lastX = static_cast<double>(m_width) / 2.0;
    m_lastY = static_cast<double>(m_height) / 2.0;
    glfwSetCursorPosCallback(m_window, Window::mouse_callback);

    // Disable vsync
    glfwSwapInterval(0);

    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        std::cerr << "Error: Window: initialize: GLEW Initialization Failed: " << glewGetErrorString(glewErr) << std::endl;
        glfwDestroyWindow(m_window);
        m_window = nullptr; // Prevent double deletion in destructor
        glfwTerminate();
        return false;
    }

    return true;
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffersAndPollEvents()
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Window::setCamera(Camera *camera)
{
    m_camera = camera;
}

bool Window::isWireframeEnabled() const
{
    return m_wireframeEnabled;
}
