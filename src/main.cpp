// =============================================================================
// main.cpp — Entry Point for the 3D Library Simulation (Phase 2)
// =============================================================================
// Demonstrates the Phase 2 shader system:
//   1. Window creation with OpenGL 3.3 Core Profile
//   2. ShaderLibrary singleton loads and manages shaders
//   3. Mesh rendering with Position + Normal + TexCoord vertex layout
//   4. Camera setup (static, looking at the origin)
//   5. MVP matrix pipeline using u_Model / u_View / u_Projection uniforms
//   6. Flat color rendering via u_Color + u_Alpha uniforms
//
// The result: a wood-brown cube slowly rotating on the Y-axis against
// a dark navy background.
//
// No raw OpenGL calls appear here — everything goes through the abstraction
// classes (Window, ShaderLibrary, Shader, Camera, Mesh, Renderer).
// =============================================================================

#include "core/Window.h"
#include "core/ShaderLibrary.h"
#include "core/Camera.h"
#include "renderer/Mesh.h"
#include "renderer/Renderer.h"
#include "utils/Logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>     // glm::perspective, glm::rotate

#include <GLFW/glfw3.h>                     // For glfwGetTime() — delta time

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

// Object color — wood-brown (library's dominant color)
const glm::vec3 CUBE_COLOR = glm::vec3(0.45f, 0.28f, 0.10f);
constexpr float CUBE_ALPHA = 1.0f;          // Fully opaque

// =============================================================================
// CreateCubeMesh — Builds a cube with proper normals and UVs
// =============================================================================
// Phase 2: Vertices now use Position + Normal + TexCoord (no per-vertex color).
// Color is controlled via the u_Color uniform in the shader.
// Each face has its own set of 4 vertices with correct face normals.
// Total: 24 vertices (4 per face × 6 faces), 36 indices.
// =============================================================================
Mesh CreateCubeMesh() {
    // ---- Define vertices for each face ----
    // Each face has 4 vertices with the correct outward-facing normal.
    // TexCoords map [0,1] across each face for future texture mapping.
    std::vector<Vertex> vertices = {
        // --- Front face (+Z) --- Normal: (0, 0, 1)
        {{ -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},  // 0: bottom-left
        {{  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},  // 1: bottom-right
        {{  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},  // 2: top-right
        {{ -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }},  // 3: top-left

        // --- Back face (-Z) --- Normal: (0, 0, -1)
        {{  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 4
        {{ -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }}, // 5
        {{ -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 6
        {{  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }}, // 7

        // --- Left face (-X) --- Normal: (-1, 0, 0)
        {{ -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 8
        {{ -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}, // 9
        {{ -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 10
        {{ -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }}, // 11

        // --- Right face (+X) --- Normal: (1, 0, 0)
        {{  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},  // 12
        {{  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},  // 13
        {{  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},  // 14
        {{  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},  // 15

        // --- Top face (+Y) --- Normal: (0, 1, 0)
        {{ -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},  // 16
        {{  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},  // 17
        {{  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},  // 18
        {{ -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},  // 19

        // --- Bottom face (-Y) --- Normal: (0, -1, 0)
        {{ -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 20
        {{  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }}, // 21
        {{  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 22
        {{ -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }}, // 23
    };

    // ---- Define the 36 indices (12 triangles, 2 per face) ----
    std::vector<unsigned int> indices = {
        // Front face
         0,  1,  2,    2,  3,  0,
        // Back face
         4,  5,  6,    6,  7,  4,
        // Left face
         8,  9, 10,   10, 11,  8,
        // Right face
        12, 13, 14,   14, 15, 12,
        // Top face
        16, 17, 18,   18, 19, 16,
        // Bottom face
        20, 21, 22,   22, 23, 20,
    };

    return Mesh(vertices, indices);
}

// =============================================================================
// main — Application entry point
// =============================================================================
int main() {
    LOG_INFO("=== 3D Library Simulation — Phase 2 ===");
    LOG_INFO("Starting application...");

    // ---- Step 1: Create the window ----
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    if (!window.GetNativeWindow()) {
        LOG_ERROR("Failed to create window — exiting");
        return -1;
    }

    // ---- Step 2: Enable depth testing ----
    glEnable(GL_DEPTH_TEST);
    LOG_INFO("Depth testing enabled");

    // ---- Step 3: Load shaders via ShaderLibrary ----
    ShaderLibrary::Get().LoadAll();
    Shader& shader = ShaderLibrary::Get().GetBasic();

    // ---- Step 4: Create the test cube mesh ----
    Mesh cube = CreateCubeMesh();

    // ---- Step 5: Create the camera ----
    Camera camera;

    // ---- Step 6: Initialize timing variables for delta time ----
    float lastFrameTime = 0.0f;
    float totalRotation = 0.0f;

    LOG_INFO("Entering main render loop...");

    // =========================================================================
    // Main Render Loop
    // =========================================================================
    while (!window.ShouldClose()) {
        // ---- Calculate delta time ----
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // ---- Poll input/window events ----
        window.PollEvents();

        // ---- Clear the screen (dark navy background) ----
        Renderer::Clear(BG_RED, BG_GREEN, BG_BLUE);

        // ---- Calculate the Model matrix (slow Y-axis rotation) ----
        totalRotation += ROTATION_SPEED * deltaTime;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, totalRotation, glm::vec3(0.0f, 1.0f, 0.0f));

        // ---- Get the View matrix from the camera ----
        glm::mat4 view = camera.GetViewMatrix();

        // ---- Calculate the Projection matrix ----
        float aspectRatio = static_cast<float>(window.GetWidth()) /
                            static_cast<float>(window.GetHeight());
        glm::mat4 projection = glm::perspective(
            glm::radians(FOV_DEGREES), aspectRatio, NEAR_PLANE, FAR_PLANE
        );

        // ---- Set shader uniforms ----
        shader.Use();
        shader.SetMat4("u_Model", model);
        shader.SetMat4("u_View", view);
        shader.SetMat4("u_Projection", projection);
        shader.SetVec3("u_Color", CUBE_COLOR);       // Wood-brown color
        shader.SetFloat("u_Alpha", CUBE_ALPHA);      // Fully opaque

        // ---- Draw the cube ----
        cube.Draw();

        // ---- Swap front and back buffers ----
        window.SwapBuffers();

        // TODO Phase 4: Process keyboard input for camera movement
        // TODO Phase 5: Replace test cube with Scene objects
    }

    // ---- Cleanup ----
    LOG_INFO("Main loop ended — cleaning up...");
    cube.Delete();

    LOG_INFO("Application exited cleanly");
    return 0;
}
