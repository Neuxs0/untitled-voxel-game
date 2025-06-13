#include "Camera.hpp"

// Constructor
Camera::Camera(glm::vec3 position, int windowWidth, int windowHeight, glm::vec3 up, float yaw, float pitch)
    : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), m_fov(FOV)
{
    m_position = position;
    m_worldUp = up;
    m_yaw = yaw;
    m_pitch = pitch;
    updateCameraVectors();
    updateProjectionMatrix(windowWidth, windowHeight);
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

// Returns the projection matrix.
const glm::mat4 &Camera::getProjectionMatrix() const
{
    return m_projectionMatrix;
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

// Updates the projection matrix. Called on window resize or FOV change.
void Camera::updateProjectionMatrix(int width, int height)
{
    if (height == 0)
        height = 1; // Prevent division by zero
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio, NEAR_PLANE, FAR_PLANE);
}

// Updates the view frustum planes from the current view and projection matrices.
void Camera::updateFrustum(const glm::mat4 &viewMatrix)
{
    const glm::mat4 vp = m_projectionMatrix * viewMatrix;
    // Transposing the matrix makes columns into rows, so vpt[i] is the i-th row of the original vp matrix.
    const glm::mat4 vpt = glm::transpose(vp);

    // Left plane:   row4 + row1
    m_frustumPlanes[0] = vpt[3] + vpt[0];
    // Right plane:  row4 - row1
    m_frustumPlanes[1] = vpt[3] - vpt[0];
    // Bottom plane: row4 + row2
    m_frustumPlanes[2] = vpt[3] + vpt[1];
    // Top plane:    row4 - row2
    m_frustumPlanes[3] = vpt[3] - vpt[1];
    // Near plane:   row4 + row3
    m_frustumPlanes[4] = vpt[3] + vpt[2];
    // Far plane:    row4 - row3
    m_frustumPlanes[5] = vpt[3] - vpt[2];

    // Normalize the plane equations.
    for (int i = 0; i < 6; ++i)
    {
        m_frustumPlanes[i] /= glm::length(glm::vec3(m_frustumPlanes[i]));
    }
}

// Checks if an Axis-Aligned Bounding Box is visible within the camera's frustum.
bool Camera::isAABBVisible(const glm::vec3 &min, const glm::vec3 &max) const
{
    for (int i = 0; i < 6; ++i)
    {
        const glm::vec4 &plane = m_frustumPlanes[i];

        // Find p-vertex (the vertex of the AABB that is most in the direction of the plane's normal)
        glm::vec3 p_vertex(
            (plane.x > 0) ? max.x : min.x,
            (plane.y > 0) ? max.y : min.y,
            (plane.z > 0) ? max.z : min.z);

        // Check if p-vertex is outside the plane (in the negative half-space)
        // If it is, the entire AABB is outside the frustum
        if (glm::dot(glm::vec3(plane), p_vertex) + plane.w < 0.0f)
        {
            return false;
        }
    }

    return true; // The AABB is inside or intersects the frustum
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
