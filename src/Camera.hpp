#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <array>

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
const float SPEED = 5.0f; // In units (blocks) per second
const float SENSITIVITY = 0.1f;
const float FOV = 85.0f;
const float NEAR_PLANE = 0.01f;
const float FAR_PLANE = 1000.0f;

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
    // Projection matrix
    glm::mat4 m_projectionMatrix;
    // Frustum planes
    std::array<glm::vec4, 6> m_frustumPlanes;

    // Calculates the front vector from the Camera's (updated) Euler Angles.
    void updateCameraVectors();

public:
    // Constructor
    Camera(glm::vec3 position, int windowWidth, int windowHeight, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
    glm::mat4 getViewMatrix() const;

    // Returns the projection matrix.
    const glm::mat4 &getProjectionMatrix() const;

    // Processes input received from any keyboard-like input system.
    void processKeyboard(CameraMovement direction, float deltaTime);

    // Processes input received from a mouse input system. Expects the offset value in both x and y.
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Updates the projection matrix. Called on window resize or FOV change.
    void updateProjectionMatrix(int width, int height);

    // Updates the view frustum planes from the current view and projection matrices.
    void updateFrustum(const glm::mat4 &viewMatrix);

    // Checks if an Axis-Aligned Bounding Box is visible within the camera's frustum.
    bool isAABBVisible(const glm::vec3 &min, const glm::vec3 &max) const;

    // Getters
    const glm::vec3 &getPosition() const;
    const glm::vec3 &getFront() const;
    float getFov() const;
};
