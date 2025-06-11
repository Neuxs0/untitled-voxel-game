#include "Camera.hpp"

// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), m_fov(FOV)
{
    m_position = position;
    m_worldUp = up;
    m_yaw = yaw;
    m_pitch = pitch;
    updateCameraVectors();
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

// Processes input received from any keyboard-like input system.
void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = m_movementSpeed * deltaTime;
    if (direction == CameraMovement::FORWARD)
        m_position += m_front * velocity;
    if (direction == CameraMovement::BACKWARD)
        m_position -= m_front * velocity;
    if (direction == CameraMovement::LEFT)
        m_position -= m_right * velocity;
    if (direction == CameraMovement::RIGHT)
        m_position += m_right * velocity;
    if (direction == CameraMovement::UP)
        m_position += m_worldUp * velocity;
    if (direction == CameraMovement::DOWN)
        m_position -= m_worldUp * velocity;
}

// Processes input received from a mouse input system. Expects the offset value in both x and y.
void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// Getters
const glm::vec3 &Camera::getPosition() const { return m_position; }
const glm::vec3 &Camera::getFront() const { return m_front; }
float Camera::getFov() const { return m_fov; }


// Calculates the front vector from the Camera's (updated) Euler Angles.
void Camera::updateCameraVectors()
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
