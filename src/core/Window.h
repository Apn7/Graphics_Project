// =============================================================================
// Window.h — GLFW Window Abstraction
// =============================================================================
// Wraps GLFW window creation, context setup, and event handling for OpenGL 3.3
// Core Profile. Provides a clean interface so the rest of the application never
// calls GLFW directly.
//
// Responsibilities:
//   - Initialize GLFW with OpenGL 3.3 Core Profile hints
//   - Create a resizable window (default 1280x720)
//   - Load GLAD after context creation
//   - Handle window resize via a callback that updates glViewport
//   - Clean up GLFW resources on destruction
// =============================================================================

#ifndef WINDOW_H
#define WINDOW_H

// Forward-declare to avoid including GLFW in the header —
// we include it only in Window.cpp
struct GLFWwindow;

#include <string>

class Window {
public:
    // =========================================================================
    // Constructor — Creates and initializes the GLFW window
    // =========================================================================
    // Parameters:
    //   width  — initial window width in pixels (default: 1280)
    //   height — initial window height in pixels (default: 720)
    //   title  — window title string
    // =========================================================================
    Window(int width = 1280, int height = 720,
           const std::string& title = "3D Library - OpenGL 3.3");

    // =========================================================================
    // Destructor — Cleans up GLFW resources
    // =========================================================================
    ~Window();

    // --- Prevent copying (GLFW window handles are not copyable) ---
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // =========================================================================
    // ShouldClose — Returns true when the user requests the window to close
    // =========================================================================
    bool ShouldClose() const;

    // =========================================================================
    // SwapBuffers — Swaps the front and back framebuffers (double-buffering)
    // =========================================================================
    void SwapBuffers();

    // =========================================================================
    // PollEvents — Processes all pending window/input events
    // =========================================================================
    void PollEvents();

    // =========================================================================
    // GetNativeWindow — Returns the raw GLFW window pointer
    // =========================================================================
    // Use sparingly — prefer the abstraction methods above.
    // =========================================================================
    GLFWwindow* GetNativeWindow() const;

    // =========================================================================
    // GetWidth / GetHeight — Return the current window dimensions
    // =========================================================================
    int GetWidth() const;
    int GetHeight() const;

private:
    GLFWwindow* m_Window;  // The underlying GLFW window handle
    int m_Width;           // Current window width in pixels
    int m_Height;          // Current window height in pixels

    // =========================================================================
    // FramebufferSizeCallback — Static callback for window resize events
    // =========================================================================
    // GLFW requires a static/free function as a callback. We use a static
    // member and retrieve the Window instance via glfwGetWindowUserPointer.
    // =========================================================================
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
};

#endif // WINDOW_H
