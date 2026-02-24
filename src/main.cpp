// =============================================================================
// main.cpp — Entry Point for the 3D Library Simulation (Phase 3)
// =============================================================================
// Demonstrates the Phase 3 primitives system:
//   1. Primitives::CreateCube() and Primitives::CreatePlane()
//   2. Transform::TRS() for building model matrices in one line
//   3. Multiple objects with different positions, scales, and colors
//   4. Depth testing with overlapping geometry
//
// The result: a gray floor plane with 5 colored cubes — including a tall
// "shelf pillar" and a rotating green cube — proving the primitive system
// works correctly.
// =============================================================================

#include "core/Window.h"
#include "core/ShaderLibrary.h"
#include "core/Camera.h"
#include "renderer/Primitives.h"
#include "renderer/Renderer.h"
#include "utils/Logger.h"
#include "utils/Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

// ---- Named Constants ----
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "3D Library - OpenGL 3.3";

// Background color (dark navy)
constexpr float BG_RED   = 0.05f;
constexpr float BG_GREEN = 0.07f;
constexpr float BG_BLUE  = 0.12f;

// Projection settings
constexpr float FOV_DEGREES = 60.0f;
constexpr float NEAR_PLANE  = 0.1f;
constexpr float FAR_PLANE   = 100.0f;

// =============================================================================
// main — Application entry point
// =============================================================================
int main() {
    LOG_INFO("=== 3D Library Simulation — Phase 3 ===");
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

    // ---- Step 4: Create primitive meshes ----
    auto cubeMesh  = Primitives::CreateCube();
    auto planeMesh = Primitives::CreatePlane();

    // ---- Step 5: Set up the camera (static for now) ----
    // TODO Phase 4: Replace hardcoded lookAt with Camera::GetViewMatrix()
    glm::mat4 view = glm::lookAt(
        glm::vec3(4.0f, 3.0f, 6.0f),   // Eye position
        glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
    );

    // ---- Step 6: Set up projection matrix ----
    float aspectRatio = static_cast<float>(window.GetWidth()) /
                        static_cast<float>(window.GetHeight());
    glm::mat4 projection = glm::perspective(
        glm::radians(FOV_DEGREES), aspectRatio, NEAR_PLANE, FAR_PLANE
    );

    // ---- Step 7: Initialize timing ----
    float lastFrameTime = 0.0f;

    LOG_INFO("Entering main render loop...");

    // =========================================================================
    // Main Render Loop
    // =========================================================================
    while (!window.ShouldClose()) {
        // ---- Calculate delta time ----
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // ---- Poll events ----
        window.PollEvents();

        // ---- Clear screen ----
        Renderer::Clear(BG_RED, BG_GREEN, BG_BLUE);

        // ---- Activate shader and set view/projection (once per frame) ----
        shader.Use();
        shader.SetMat4("u_View", view);
        shader.SetMat4("u_Projection", projection);
        shader.SetFloat("u_Alpha", 1.0f);

        // ============================================================
        // Draw scene objects — each with its own transform and color
        // ============================================================

        // Floor plane — scaled large, light gray
        shader.SetMat4("u_Model", Transform::TRS({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {10.0f, 1.0f, 8.0f}));
        shader.SetVec3("u_Color", glm::vec3(0.78f, 0.78f, 0.80f));
        planeMesh->Draw();

        // Cube 1 — dark brown (shelf color)
        shader.SetMat4("u_Model", Transform::TRS({-2.0f, 0.5f, 0.0f}));
        shader.SetVec3("u_Color", glm::vec3(0.35f, 0.20f, 0.08f));
        cubeMesh->Draw();

        // Cube 2 — cream/white (wall color)
        shader.SetMat4("u_Model", Transform::TRS({0.0f, 0.5f, 0.0f}));
        shader.SetVec3("u_Color", glm::vec3(0.92f, 0.90f, 0.85f));
        cubeMesh->Draw();

        // Cube 3 — navy blue (chair cushion color)
        shader.SetMat4("u_Model", Transform::TRS({2.0f, 0.5f, 0.0f}));
        shader.SetVec3("u_Color", glm::vec3(0.12f, 0.18f, 0.55f));
        cubeMesh->Draw();

        // Cube 4 — tall shelf (non-uniform scale demonstration)
        shader.SetMat4("u_Model", Transform::TRS({-4.0f, 1.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.5f, 3.0f, 0.4f}));
        shader.SetVec3("u_Color", glm::vec3(0.35f, 0.20f, 0.08f));
        cubeMesh->Draw();

        // Cube 5 — rotating green cube (confirms deltaTime accumulation)
        static float angle = 0.0f;
        angle += 30.0f * deltaTime;
        shader.SetMat4("u_Model", Transform::TRS({4.0f, 0.5f, 0.0f}, {0.0f, angle, 0.0f}));
        shader.SetVec3("u_Color", glm::vec3(0.15f, 0.55f, 0.35f));
        cubeMesh->Draw();

        // ---- Swap buffers ----
        window.SwapBuffers();

        // TODO Phase 4: Process keyboard input for camera movement
        // TODO Phase 5: Replace demo cubes with full Scene objects
    }

    // ---- Cleanup ----
    LOG_INFO("Main loop ended — cleaning up...");
    // cubeMesh and planeMesh are unique_ptrs — cleaned up automatically

    LOG_INFO("Application exited cleanly");
    return 0;
}
