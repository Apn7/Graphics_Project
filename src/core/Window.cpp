// =============================================================================
// Window.cpp — GLFW Window Implementation
// =============================================================================
// Implements the Window class: initializes GLFW, creates an OpenGL 3.3 Core
// Profile context, loads GLAD, and handles window resize events.
// =============================================================================

#include "core/Window.h"
#include "utils/Logger.h"

// Include GLAD before GLFW — GLAD provides the OpenGL function pointers
// that GLFW needs. This ordering is critical.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// =============================================================================
// Constructor — Initializes GLFW, creates window, loads GLAD
// =============================================================================
Window::Window(int width, int height, const std::string& title)
    : m_Window(nullptr), m_Width(width), m_Height(height)
{
    // ---- Step 1: Initialize the GLFW library ----
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW!");
        return;
    }
    LOG_INFO("GLFW initialized successfully");

    // ---- Step 2: Set OpenGL version hints (3.3 Core Profile) ----
    // These hints tell GLFW what kind of OpenGL context to create.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);     // OpenGL 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);     // OpenGL x.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Core = no legacy
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);         // Allow window resizing

    // macOS requires this hint for OpenGL 3.2+ Core Profile to work
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // ---- Step 3: Create the GLFW window ----
    m_Window = glfwCreateWindow(m_Width, m_Height, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        LOG_ERROR("Failed to create GLFW window!");
        glfwTerminate();
        return;
    }
    LOG_INFO("Window created: " + std::to_string(m_Width) + "x" + std::to_string(m_Height));

    // ---- Step 4: Make this window's OpenGL context current ----
    // All subsequent OpenGL calls will target this window's context.
    glfwMakeContextCurrent(m_Window);

    // ---- Step 5: Load GLAD (OpenGL function pointers) ----
    // GLAD must be loaded AFTER the context is made current.
    // glfwGetProcAddress is the function GLAD uses to find OpenGL functions.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD! OpenGL functions are unavailable.");
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        m_Window = nullptr;
        return;
    }
    LOG_INFO("GLAD loaded successfully — OpenGL is ready");

    // ---- Step 6: Log the OpenGL version for debugging ----
    LOG_INFO(std::string("OpenGL Version: ") +
             reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    LOG_INFO(std::string("GPU Renderer: ") +
             reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    // ---- Step 7: Store a pointer to this Window instance on the GLFW window ----
    // This allows our static callback function to access the Window object.
    glfwSetWindowUserPointer(m_Window, this);

    // ---- Step 8: Register the framebuffer resize callback ----
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

    // ---- Step 9: Set the initial viewport to match the window size ----
    glViewport(0, 0, m_Width, m_Height);
}

// =============================================================================
// Destructor — Cleans up GLFW resources
// =============================================================================
Window::~Window() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);   // Destroy the window handle
        LOG_INFO("Window destroyed");
    }
    glfwTerminate();                   // Shut down GLFW entirely
    LOG_INFO("GLFW terminated");
}

// =============================================================================
// ShouldClose — Checks if the user requested the window to close
// =============================================================================
bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

// =============================================================================
// SwapBuffers — Swaps the double-buffered framebuffers
// =============================================================================
// OpenGL renders to the "back" buffer while the "front" buffer is displayed.
// SwapBuffers makes the back buffer visible and starts a new frame on the old front.
// =============================================================================
void Window::SwapBuffers() {
    glfwSwapBuffers(m_Window);
}

// =============================================================================
// PollEvents — Processes all pending input/window events
// =============================================================================
// Must be called every frame to keep the window responsive.
// =============================================================================
void Window::PollEvents() {
    glfwPollEvents();
}

// =============================================================================
// GetNativeWindow — Returns the raw GLFW window pointer
// =============================================================================
GLFWwindow* Window::GetNativeWindow() const {
    return m_Window;
}

// =============================================================================
// GetWidth / GetHeight — Return current window dimensions
// =============================================================================
int Window::GetWidth() const {
    return m_Width;
}

int Window::GetHeight() const {
    return m_Height;
}

// =============================================================================
// FramebufferSizeCallback — Called by GLFW when the window is resized
// =============================================================================
// Updates the OpenGL viewport to match the new window size, and stores
// the new dimensions in the Window object.
// =============================================================================
void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Retrieve our Window instance from the GLFW user pointer
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_Width = width;
        win->m_Height = height;
    }

    // Update the OpenGL viewport to cover the entire new window area
    glViewport(0, 0, width, height);
    LOG_INFO("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
}
