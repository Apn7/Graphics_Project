// =============================================================================
// Camera.h — FPS-Style Camera (Phase 1 Stub)
// =============================================================================
// Defines the camera interface for the 3D Library. In Phase 1, only the basic
// view matrix is implemented. Full keyboard/mouse controls will be added
// in Phase 4.
//
// Default position: (0.0, 1.5, 5.0) — slightly above and behind the origin,
// looking toward the center of the scene.
// =============================================================================

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>          // glm::vec3, glm::mat4
#include <glm/gtc/matrix_transform.hpp>  // glm::lookAt

// ---- Default Camera Values ----
// Named constants instead of magic numbers (per project rules)
namespace CameraDefaults {
    const glm::vec3 POSITION = glm::vec3(0.0f, 1.5f, 5.0f);   // Starting position
    const glm::vec3 FRONT    = glm::vec3(0.0f, 0.0f, -1.0f);  // Looking toward -Z
    const glm::vec3 UP       = glm::vec3(0.0f, 1.0f, 0.0f);   // World up direction

    // TODO Phase 4: Add default values for movement speed, mouse sensitivity, zoom
}

class Camera {
public:
    // =========================================================================
    // Constructor — Sets the camera to its default position and orientation
    // =========================================================================
    Camera(glm::vec3 position = CameraDefaults::POSITION,
           glm::vec3 front = CameraDefaults::FRONT,
           glm::vec3 up = CameraDefaults::UP);

    // =========================================================================
    // GetViewMatrix — Returns the view matrix calculated from camera vectors
    // =========================================================================
    // Uses glm::lookAt to create a view matrix that transforms world-space
    // coordinates into camera-space (view-space) coordinates.
    // =========================================================================
    glm::mat4 GetViewMatrix() const;

    // =========================================================================
    // GetPosition — Returns the camera's current world-space position
    // =========================================================================
    glm::vec3 GetPosition() const;

    // TODO Phase 4: void ProcessKeyboard(CameraDirection direction, float deltaTime);
    // TODO Phase 4: void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    // TODO Phase 4: void ProcessMouseScroll(float yOffset);

private:
    glm::vec3 m_Position;   // Camera position in world space
    glm::vec3 m_Front;      // Direction the camera is looking (normalized)
    glm::vec3 m_Up;         // Camera's up vector

    // TODO Phase 4: float m_Yaw, m_Pitch;
    // TODO Phase 4: float m_MovementSpeed, m_MouseSensitivity, m_Zoom;
    // TODO Phase 4: void UpdateCameraVectors(); — recalculates Front from Yaw/Pitch
};

#endif // CAMERA_H
