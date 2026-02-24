// =============================================================================
// main.cpp — Entry Point for the 3D Library Simulation
// =============================================================================
// This is the main entry point for the application. It demonstrates all
// Phase 1 systems working together:
//   1. Window creation with OpenGL 3.3 Core Profile
//   2. Shader loading and compilation
//   3. Mesh rendering (a colored cube)
//   4. Camera setup (static, looking at the origin)
//   5. MVP matrix pipeline (Model × View × Projection)
//   6. Main render loop with delta time and smooth rotation
//
// The result: a multi-colored cube slowly rotating on the Y-axis against
// a dark navy background.
//
// No raw OpenGL calls appear here — everything goes through the abstraction
// classes (Window, Shader, Camera, Mesh, Renderer).
// =============================================================================

#include "core/Window.h"
#include "core/Shader.h"
#include "core/Camera.h"
#include "renderer/Mesh.h"
#include "renderer/Renderer.h"
#include "utils/Logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>     // glm::perspective, glm::rotate, glm::translate

#include <GLFW/glfw3.h>                     // For glfwGetTime() — delta time calculation

#include <vector>

// ---- Named Constants ----
// Per project rules: no magic numbers — use named constants

// Window settings
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "3D Library - OpenGL 3.3";

// Background color (dark navy)
constexpr float BG_RED   = 0.05f;
constexpr float BG_GREEN = 0.07f;
constexpr float BG_BLUE  = 0.12f;

// Projection settings
constexpr float FOV_DEGREES  = 45.0f;      // Field of view in degrees
constexpr float NEAR_PLANE   = 0.1f;       // Near clipping plane
constexpr float FAR_PLANE    = 100.0f;     // Far clipping plane

// Rotation speed (radians per second)
constexpr float ROTATION_SPEED = 0.5f;

// Shader file paths
const std::string VERTEX_SHADER_PATH   = "shaders/basic.vert";
const std::string FRAGMENT_SHADER_PATH = "shaders/basic.frag";

// =============================================================================
// CreateCubeMesh — Builds a colored cube with 8 unique vertices and 36 indices
// =============================================================================
// Each vertex has a unique color so the cube displays a vibrant, multi-colored
// appearance. The 36 indices define 12 triangles (2 per face × 6 faces).
// =============================================================================
Mesh CreateCubeMesh() {
    // ---- Define the 8 corners of the cube ----
    // Each vertex has a Position (x, y, z) and a Color (r, g, b)
    // The cube is centered at the origin with side length 1.0
    std::vector<Vertex> vertices = {
        // Position                      // Color
        // Front face vertices (z = +0.5)
        {{ -0.5f, -0.5f,  0.5f },  { 1.0f, 0.0f, 0.0f }},  // 0: bottom-left  — Red
        {{  0.5f, -0.5f,  0.5f },  { 0.0f, 1.0f, 0.0f }},  // 1: bottom-right — Green
        {{  0.5f,  0.5f,  0.5f },  { 0.0f, 0.0f, 1.0f }},  // 2: top-right    — Blue
        {{ -0.5f,  0.5f,  0.5f },  { 1.0f, 1.0f, 0.0f }},  // 3: top-left     — Yellow

        // Back face vertices (z = -0.5)
        {{ -0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f, 1.0f }},  // 4: bottom-left  — Magenta
        {{  0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f, 1.0f }},  // 5: bottom-right — Cyan
        {{  0.5f,  0.5f, -0.5f },  { 1.0f, 0.5f, 0.0f }},  // 6: top-right    — Orange
        {{ -0.5f,  0.5f, -0.5f },  { 0.5f, 0.0f, 1.0f }},  // 7: top-left     — Purple
    };

    // ---- Define the 36 indices (12 triangles, 2 per face) ----
    // Each face is defined as two triangles using counter-clockwise winding
    std::vector<unsigned int> indices = {
        // Front face  (+Z)
        0, 1, 2,    2, 3, 0,
        // Back face   (-Z)
        5, 4, 7,    7, 6, 5,
        // Left face   (-X)
        4, 0, 3,    3, 7, 4,
        // Right face  (+X)
        1, 5, 6,    6, 2, 1,
        // Top face    (+Y)
        3, 2, 6,    6, 7, 3,
        // Bottom face (-Y)
        4, 5, 1,    1, 0, 4,
    };

    return Mesh(vertices, indices);
}

// =============================================================================
// main — Application entry point
// =============================================================================
int main() {
    LOG_INFO("=== 3D Library Simulation — Phase 1 ===");
    LOG_INFO("Starting application...");

    // ---- Step 1: Create the window ----
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    if (!window.GetNativeWindow()) {
        LOG_ERROR("Failed to create window — exiting");
        return -1;
    }

    // ---- Step 2: Enable depth testing ----
    // Without depth testing, OpenGL draws triangles in submission order,
    // causing back faces to appear in front of closer faces.
    glEnable(GL_DEPTH_TEST);
    LOG_INFO("Depth testing enabled");

    // ---- Step 3: Load and compile shaders ----
    Shader shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
    if (shader.GetID() == 0) {
        LOG_ERROR("Shader creation failed — exiting");
        return -1;
    }

    // ---- Step 4: Create the test cube mesh ----
    Mesh cube = CreateCubeMesh();

    // ---- Step 5: Create the camera ----
    // Default position: (0, 1.5, 5) looking toward the origin
    Camera camera;

    // ---- Step 6: Initialize timing variables for delta time ----
    // deltaTime = time between current frame and last frame
    // Used to make rotation frame-rate independent
    float lastFrameTime = 0.0f;      // Time of the previous frame
    float totalRotation = 0.0f;       // Accumulated rotation angle (radians)

    LOG_INFO("Entering main render loop...");

    // =========================================================================
    // Main Render Loop
    // =========================================================================
    // Runs every frame until the user closes the window.
    // Each iteration: poll events → clear screen → calculate transforms →
    // set uniforms → draw → swap buffers
    // =========================================================================
    while (!window.ShouldClose()) {
        // ---- Calculate delta time ----
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // ---- Poll input/window events ----
        window.PollEvents();

        // ---- Clear the screen ----
        // Dark navy background as specified in the prompt
        Renderer::Clear(BG_RED, BG_GREEN, BG_BLUE);

        // ---- Calculate the Model matrix ----
        // The model matrix transforms the cube from model space to world space.
        // Here we apply a slow Y-axis rotation using accumulated time.
        totalRotation += ROTATION_SPEED * deltaTime;
        glm::mat4 model = glm::mat4(1.0f);     // Start with identity matrix
        model = glm::rotate(
            model,
            totalRotation,                       // Rotation angle in radians
            glm::vec3(0.0f, 1.0f, 0.0f)         // Rotate around Y-axis
        );

        // ---- Get the View matrix from the camera ----
        glm::mat4 view = camera.GetViewMatrix();

        // ---- Calculate the Projection matrix ----
        // Perspective projection: objects farther away appear smaller
        float aspectRatio = static_cast<float>(window.GetWidth()) /
                            static_cast<float>(window.GetHeight());
        glm::mat4 projection = glm::perspective(
            glm::radians(FOV_DEGREES),    // Vertical field of view
            aspectRatio,                   // Aspect ratio (width / height)
            NEAR_PLANE,                    // Near clipping plane
            FAR_PLANE                      // Far clipping plane
        );

        // ---- Set shader uniforms ----
        shader.Use();                             // Activate the shader program
        shader.SetMat4("model", model);           // Upload model matrix
        shader.SetMat4("view", view);             // Upload view matrix
        shader.SetMat4("projection", projection); // Upload projection matrix

        // ---- Draw the cube ----
        cube.Draw();

        // ---- Swap front and back buffers ----
        window.SwapBuffers();

        // TODO Phase 4: Process keyboard input for camera movement
        // TODO Phase 5: Replace test cube with Scene objects
    }

    // ---- Cleanup ----
    LOG_INFO("Main loop ended — cleaning up...");
    cube.Delete();      // Explicitly free GPU memory (also done by destructor)
    // Window, Shader, Camera destructors handle their own cleanup automatically

    LOG_INFO("Application exited cleanly");
    return 0;
}
