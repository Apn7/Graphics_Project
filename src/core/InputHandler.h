// =============================================================================
// InputHandler.h — Centralized Input System (Phase 4)
// =============================================================================
// Handles all GLFW input: keyboard, mouse, scroll wheel.
// Keeps main.cpp clean — no raw glfwGetKey calls outside this class.
//
// Usage:
//   InputHandler::Init(window, &camera);           // Once at startup
//   InputHandler::ProcessContinuousInput(window, &camera, dt);  // Every frame
// =============================================================================

#pragma once

#include "core/Camera.h"

// Forward-declare GLFWwindow so we don't pull in GLFW (and therefore the system
// OpenGL header) in every translation unit that includes this header.
// The full #include <GLFW/glfw3.h> is in InputHandler.cpp, after glad.h.
struct GLFWwindow;


class InputHandler {
public:
    // =========================================================================
    // Init — Register GLFW callbacks and capture the mouse
    // =========================================================================
    // Call once at startup after window creation.
    // =========================================================================
    static void Init(GLFWwindow* window, Camera* camera);

    // =========================================================================
    // ProcessContinuousInput — Poll held-down keys for smooth movement
    // =========================================================================
    // Call every frame in the main loop.
    // =========================================================================
    static void ProcessContinuousInput(GLFWwindow* window, Camera* camera,
                                       float deltaTime);

    // =========================================================================
    // State getters
    // =========================================================================
    static bool IsMouseCaptured() { return s_MouseCaptured; }

    // TODO Phase 9: Add F key callback for ceiling fan toggle

private:
    // ---- GLFW Callbacks ----
    static void MouseCallback(GLFWwindow* w, double x, double y);
    static void ScrollCallback(GLFWwindow* w, double xOff, double yOff);
    static void KeyCallback(GLFWwindow* w, int key, int scancode,
                            int action, int mods);
    static void FramebufferSizeCallback(GLFWwindow* w, int width, int height);

    // ---- State ----
    static Camera* s_Camera;
    static float   s_LastMouseX;
    static float   s_LastMouseY;
    static bool    s_FirstMouse;     // Prevents jump on first mouse move
    static bool    s_MouseCaptured;  // True when cursor is hidden/locked
    static bool    s_SprintHeld;     // True when Left Shift is down
};
