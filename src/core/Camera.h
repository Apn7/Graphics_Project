// =============================================================================
// Camera.h — First-Person Camera System (Phase 4)
// =============================================================================
// Fully interactive FPS camera with:
//   - WASD + Q/E movement (with sprint)
//   - Mouse look (yaw/pitch from euler angles)
//   - Scroll wheel zoom (FOV)
//   - Y-lock at standing height (1.7 units = eye level)
//
// Usage:
//   Camera camera(glm::vec3(0, 1.7f, 5), -90.0f, 0.0f);
//   camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
//   camera.ProcessMouseMovement(xOffset, yOffset);
//   glm::mat4 view = camera.GetViewMatrix();
// =============================================================================

#pragma once

#include <glm/glm.hpp>

// Movement direction enum — cleaner than raw key constants in camera code
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,        // Q key — fly up (useful for inspecting scene top-down)
    DOWN       // E key — fly down
};

class Camera {
public:
    // =========================================================================
    // Constructor
    // =========================================================================
    // Default starting position: inside the library, near the entrance,
    // looking toward the center of the room.
    // Position: (0, 1.7, 5) — standing height, near front wall
    // Yaw: -90 degrees (facing -Z, toward back of room)
    // Pitch: 0 degrees (level horizon)
    // =========================================================================
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 1.7f, 5.0f),
        float yaw          = -90.0f,
        float pitch        = 0.0f
    );

    // =========================================================================
    // Core Output
    // =========================================================================

    // Returns the view matrix for this frame. Upload to shader as u_View.
    glm::mat4 GetViewMatrix() const;

    // Returns current camera world-space position.
    // TODO Phase 6: GetPosition() is used as u_ViewPos uniform for specular highlights
    glm::vec3 GetPosition() const { return m_Position; }

    // Returns the direction the camera is currently facing (normalized)
    glm::vec3 GetFront() const { return m_Front; }

    // Returns current yaw angle (degrees) — for title bar display
    float GetYaw() const { return m_Yaw; }

    // Returns current FOV (degrees) — use this for projection matrix
    float GetFOV() const { return m_FOV; }

    // =========================================================================
    // Input Handlers (called every frame from InputHandler)
    // =========================================================================

    // Handle WASD + QE keyboard movement.
    // deltaTime ensures speed is frame-rate independent.
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Handle mouse look. xOffset/yOffset are pixel deltas since last frame.
    // constrainPitch: if true, prevents flipping upside-down (keep true).
    void ProcessMouseMovement(float xOffset, float yOffset,
                              bool constrainPitch = true);

    // Handle scroll wheel for FOV zoom
    void ProcessMouseScroll(float yOffset);

    // =========================================================================
    // Settings (tunable at runtime)
    // =========================================================================
    float MovementSpeed    = 4.0f;   // Units per second — library scale ~12x8 units
    float MouseSensitivity = 0.08f;  // Degrees per pixel
    float SprintMultiplier = 2.5f;   // Applied when Left Shift is held

private:
    // Camera state
    glm::vec3 m_Position;     // World-space position
    glm::vec3 m_Front;        // Direction camera faces (computed from yaw/pitch)
    glm::vec3 m_Up;           // Camera-local up vector (computed)
    glm::vec3 m_Right;        // Camera-local right vector (computed)
    glm::vec3 m_WorldUp;      // Global up (always 0,1,0)

    // Euler angles (degrees)
    float m_Yaw;              // Left/right rotation around Y axis
    float m_Pitch;            // Up/down rotation, clamped to ±89°

    // FOV for projection matrix (degrees)
    float m_FOV = 60.0f;

    // Recomputes m_Front, m_Right, m_Up from current m_Yaw and m_Pitch.
    // Must be called after any yaw/pitch change.
    void UpdateCameraVectors();
};
