// =============================================================================
// Camera.cpp — Camera Implementation (Phase 1 Stub)
// =============================================================================
// Implements the basic camera functionality for Phase 1:
//   - Store position, front, and up vectors
//   - Return a view matrix via glm::lookAt
//
// Full movement and mouse-look will be added in Phase 4.
// =============================================================================

#include "core/Camera.h"
#include "utils/Logger.h"

// =============================================================================
// Constructor — Initializes camera with given position and orientation
// =============================================================================
Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up)
    : m_Position(position), m_Front(front), m_Up(up)
{
    LOG_INFO("Camera created at position (" +
             std::to_string(m_Position.x) + ", " +
             std::to_string(m_Position.y) + ", " +
             std::to_string(m_Position.z) + ")");

    // TODO Phase 4: Initialize yaw, pitch, speed, sensitivity, zoom
    // TODO Phase 4: Call UpdateCameraVectors() to calculate initial front vector from yaw/pitch
}

// =============================================================================
// GetViewMatrix — Calculates and returns the view matrix
// =============================================================================
// glm::lookAt creates a view matrix given:
//   1. Camera position (where the camera is)
//   2. Target position (where the camera is looking = position + front)
//   3. Up vector (which direction is "up" for the camera)
// =============================================================================
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(
        m_Position,                 // Eye position
        m_Position + m_Front,       // Look-at target (position + direction)
        m_Up                        // Up direction
    );
}

// =============================================================================
// GetPosition — Returns the camera's world-space position
// =============================================================================
glm::vec3 Camera::GetPosition() const {
    return m_Position;
}

// TODO Phase 4: Implement ProcessKeyboard() — WASD movement
// TODO Phase 4: Implement ProcessMouseMovement() — look around
// TODO Phase 4: Implement ProcessMouseScroll() — zoom (change FOV)
// TODO Phase 4: Implement UpdateCameraVectors() — recalc front from yaw/pitch
