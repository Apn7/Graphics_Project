// =============================================================================
// InputHandler.cpp — Input System Implementation (Phase 4)
// =============================================================================
// Centralizes all GLFW input handling:
//   - WASD movement (continuous polling)
//   - Q/E up/down (fly mode)
//   - Mouse look (callback-driven)
//   - Scroll wheel zoom (callback-driven)
//   - Tab: toggle cursor capture
//   - Left Shift: sprint
//   - ESC: close window
// =============================================================================

#include <glad/glad.h>      // MUST come before any GLFW/OpenGL includes
#include <GLFW/glfw3.h>     // Must come after glad

#include "core/InputHandler.h"
#include "utils/Logger.h"

// =============================================================================
// Static member initialization
// =============================================================================
Camera* InputHandler::s_Camera       = nullptr;
float   InputHandler::s_LastMouseX   = 640.0f;   // Center of 1280-wide window
float   InputHandler::s_LastMouseY   = 360.0f;   // Center of 720-high window
bool    InputHandler::s_FirstMouse   = true;
bool    InputHandler::s_MouseCaptured = true;
bool    InputHandler::s_SprintHeld   = false;

// =============================================================================
// Init — Register GLFW callbacks and capture mouse
// =============================================================================
void InputHandler::Init(GLFWwindow* window, Camera* camera) {
    s_Camera = camera;

    // Register GLFW callbacks
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Capture the mouse cursor on startup (FPS mode)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    s_MouseCaptured = true;

    LOG_INFO("InputHandler initialized — mouse captured, WASD/QE/Shift/Tab/ESC active");
}

// =============================================================================
// ProcessContinuousInput — Poll held keys every frame
// =============================================================================
// Uses glfwGetKey for smooth, continuous movement.
// Sprint multiplier is applied when Left Shift is held.
// =============================================================================
void InputHandler::ProcessContinuousInput(GLFWwindow* window, Camera* camera,
                                           float deltaTime) {
    // Apply sprint multiplier temporarily
    float savedSpeed = camera->MovementSpeed;
    if (s_SprintHeld) {
        camera->MovementSpeed *= camera->SprintMultiplier;
    }

    // WASD movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(CameraMovement::RIGHT, deltaTime);

    // Q/E vertical movement (fly up/down for inspection)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera->ProcessKeyboard(CameraMovement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera->ProcessKeyboard(CameraMovement::DOWN, deltaTime);

    // Restore original speed
    camera->MovementSpeed = savedSpeed;

    // ESC to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// =============================================================================
// MouseCallback — Handle mouse movement for camera look
// =============================================================================
void InputHandler::MouseCallback(GLFWwindow* w, double x, double y) {
    // First mouse move: initialize position to prevent sudden camera jump
    if (s_FirstMouse) {
        s_LastMouseX = static_cast<float>(x);
        s_LastMouseY = static_cast<float>(y);
        s_FirstMouse = false;
    }

    // Calculate pixel deltas
    float xOffset = static_cast<float>(x) - s_LastMouseX;   // +x = look right
    float yOffset = s_LastMouseY - static_cast<float>(y);    // Inverted: +y = look up

    s_LastMouseX = static_cast<float>(x);
    s_LastMouseY = static_cast<float>(y);

    // Only process mouse look when cursor is captured
    if (s_MouseCaptured && s_Camera) {
        s_Camera->ProcessMouseMovement(xOffset, yOffset);
    }
}

// =============================================================================
// ScrollCallback — Handle scroll wheel for FOV zoom
// =============================================================================
void InputHandler::ScrollCallback(GLFWwindow* w, double xOff, double yOff) {
    if (s_Camera) {
        s_Camera->ProcessMouseScroll(static_cast<float>(yOff));
    }
}

// =============================================================================
// KeyCallback — Handle one-shot key events (not held keys)
// =============================================================================
void InputHandler::KeyCallback(GLFWwindow* w, int key, int scancode,
                                int action, int mods) {
    // Tab: toggle mouse capture (show/hide cursor)
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        s_MouseCaptured = !s_MouseCaptured;
        glfwSetInputMode(w, GLFW_CURSOR,
            s_MouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        s_FirstMouse = true;    // Reset to prevent jump when re-capturing

        if (s_MouseCaptured) {
            LOG_INFO("Mouse captured (FPS mode)");
        } else {
            LOG_INFO("Mouse released (cursor visible)");
        }
    }

    // Left Shift: track sprint state
    if (key == GLFW_KEY_LEFT_SHIFT) {
        s_SprintHeld = (action != GLFW_RELEASE);
    }

    // TODO Phase 8: Add L key (toggle lights), N key (night mode) here
}

// =============================================================================
// FramebufferSizeCallback — Handle window resize
// =============================================================================
void InputHandler::FramebufferSizeCallback(GLFWwindow* w, int width, int height) {
    glViewport(0, 0, width, height);
    // TODO: Notify main loop to rebuild projection matrix with new aspect ratio
}
