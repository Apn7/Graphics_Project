// =============================================================================
// Camera.cpp — First-Person Camera Implementation (Phase 4)
// =============================================================================
// Implements the fully interactive FPS camera:
//   - Euler angles (yaw/pitch) → Cartesian direction vectors
//   - WASD + Q/E movement with deltaTime
//   - Mouse look with configurable sensitivity
//   - Scroll wheel zoom (FOV adjustment)
//   - Y-lock at standing height (1.7 units)
// =============================================================================

#include "core/Camera.h"
#include "utils/Logger.h"

#include <glm/gtc/matrix_transform.hpp>     // glm::lookAt, glm::radians
#include <cmath>                            // cos, sin

// =============================================================================
// Constructor
// =============================================================================
Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : m_Position(position),
      m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      m_Up(glm::vec3(0.0f, 1.0f, 0.0f)),
      m_Right(glm::vec3(1.0f, 0.0f, 0.0f)),
      m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
      m_Yaw(yaw),
      m_Pitch(pitch),
      m_FOV(60.0f)
{
    // Compute initial direction vectors from yaw/pitch
    UpdateCameraVectors();

    LOG_INFO("Camera created at (" +
             std::to_string(m_Position.x) + ", " +
             std::to_string(m_Position.y) + ", " +
             std::to_string(m_Position.z) + ") | Yaw: " +
             std::to_string(m_Yaw) + " | Pitch: " +
             std::to_string(m_Pitch));
}

// =============================================================================
// GetViewMatrix — Returns the view matrix calculated from camera vectors
// =============================================================================
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

// =============================================================================
// ProcessKeyboard — Handle WASD + Q/E movement
// =============================================================================
// Movement is frame-rate independent via deltaTime.
// Y-position is locked to standing height (1.7 units) for WASD/QE.
// Remove the Y-lock line for free-fly mode.
// =============================================================================
void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;

    if (direction == CameraMovement::FORWARD)  m_Position += m_Front * velocity;
    if (direction == CameraMovement::BACKWARD) m_Position -= m_Front * velocity;
    if (direction == CameraMovement::LEFT)     m_Position -= m_Right * velocity;
    if (direction == CameraMovement::RIGHT)    m_Position += m_Right * velocity;
    if (direction == CameraMovement::UP)       m_Position += m_WorldUp * velocity;
    if (direction == CameraMovement::DOWN)     m_Position -= m_WorldUp * velocity;

    // Y starts at 1.7 (set in constructor) — no lock, fully free movement
}

// =============================================================================
// ProcessMouseMovement — Handle mouse look (yaw + pitch)
// =============================================================================
// xOffset/yOffset are pixel deltas since last frame.
// constrainPitch prevents the camera from flipping upside-down.
// =============================================================================
void Camera::ProcessMouseMovement(float xOffset, float yOffset,
                                   bool constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    m_Yaw   += xOffset;
    m_Pitch += yOffset;

    if (constrainPitch) {
        // Prevent camera from flipping past vertical (gimbal lock at 90°)
        if (m_Pitch >  89.0f) m_Pitch =  89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }

    // Recompute direction vectors from new yaw/pitch
    UpdateCameraVectors();
}

// =============================================================================
// ProcessMouseScroll — Handle scroll wheel zoom (FOV change)
// =============================================================================
void Camera::ProcessMouseScroll(float yOffset) {
    m_FOV -= yOffset;                        // Scroll up = zoom in (smaller FOV)
    if (m_FOV < 20.0f) m_FOV = 20.0f;      // Min zoom
    if (m_FOV > 90.0f) m_FOV = 90.0f;      // Max zoom (wide angle)
}

// =============================================================================
// UpdateCameraVectors — Recomputes Front, Right, Up from Yaw and Pitch
// =============================================================================
// Converts spherical coordinates (yaw, pitch) to a Cartesian direction vector.
// Must be called every time yaw or pitch changes.
// =============================================================================
void Camera::UpdateCameraVectors() {
    // Convert spherical (yaw, pitch) to Cartesian direction vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Recompute Right and Up using cross products
    // Right = Front × WorldUp (normalized)
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));

    // Up = Right × Front (normalized)
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
