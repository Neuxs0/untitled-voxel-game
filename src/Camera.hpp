#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Defines several possible options for camera movement.
// Used as an abstraction to stay away from window-system specific input methods.
enum class CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 3.7f;
const float SENSITIVITY = 0.1f;
const float FOV = 85.0f;

/**
 * @class Camera
 * @brief An abstract camera class that processes input and calculates the corresponding Euler Angles,
 * Vectors and Matrices for use in OpenGL.
 *
 * This class provides a way to handle camera movement and orientation. It processes keyboard and mouse
 * input to update its position and direction, and can provide a corresponding view matrix.
 */
class Camera
{
private:
    // Camera Attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    // Euler Angles
    float m_yaw;
    float m_pitch;
    // Camera options
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_fov;

    // Calculates the front vector from the Camera's (updated) Euler Angles.
    void updateCameraVectors();

public:
    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
    glm::mat4 getViewMatrix() const;

    // Processes input received from any keyboard-like input system.
    void processKeyboard(CameraMovement direction, float deltaTime);

    // Processes input received from a mouse input system. Expects the offset value in both x and y.
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Getters
    const glm::vec3 &getPosition() const;
    const glm::vec3 &getFront() const;
    float getFov() const;
};
