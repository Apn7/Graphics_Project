// =============================================================================
// main.cpp — Entry Point for the 3D Library Simulation (Phase 4)
// =============================================================================
// Demonstrates the Phase 4 camera system:
//   1. Full FPS camera — WASD movement, mouse look, scroll zoom
//   2. InputHandler — centralized GLFW input (no raw glfwGetKey in main)
//   3. Live window title with FPS + camera position + yaw
//   4. Phase 3 demo scene (5 cubes + floor) still intact
//
// Controls:
//   WASD      — Move forward/backward/left/right
//   Q/E       — Fly up/down
//   Mouse     — Look around
//   Scroll    — Zoom in/out (FOV)
//   Left Shift — Sprint (2.5x speed)
//   Tab       — Toggle cursor capture
//   ESC       — Exit
// =============================================================================

#include "core/Window.h"
#include "core/ShaderLibrary.h"
#include "core/Camera.h"
#include "core/InputHandler.h"
#include "renderer/Primitives.h"
#include "renderer/Renderer.h"
#include "utils/Logger.h"
#include "utils/Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include <string>

// ---- Named Constants ----
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const std::string WINDOW_TITLE = "3D Library - OpenGL 3.3";

// Background color (dark navy)
constexpr float BG_RED   = 0.05f;
constexpr float BG_GREEN = 0.07f;
constexpr float BG_BLUE  = 0.12f;

// Projection settings
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE  = 100.0f;

// =============================================================================
// main — Application entry point
// =============================================================================
int main() {
    LOG_INFO("=== 3D Library Simulation — Phase 4 ===");
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

    // ---- Step 5: Create the camera (FPS, starting near entrance) ----
    Camera camera(glm::vec3(0.0f, 1.7f, 5.0f), -90.0f, 0.0f);

    // ---- Step 6: Initialize input handler ----
    InputHandler::Init(window.GetNativeWindow(), &camera);

    // ---- Step 7: Initialize timing ----
    float lastFrameTime = 0.0f;

    LOG_INFO("Entering main render loop...");
    LOG_INFO("Controls: WASD=move, QE=up/down, Mouse=look, Scroll=zoom, Shift=sprint, Tab=cursor, ESC=exit");

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

        // ---- Process input (WASD, sprint, ESC) ----
        InputHandler::ProcessContinuousInput(window.GetNativeWindow(), &camera, deltaTime);

        // ---- Update window title with live info ----
        float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
        glm::vec3 pos = camera.GetPosition();
        std::string title = "3D Library | FPS: " + std::to_string(static_cast<int>(fps))
            + " | Pos: ("
            + std::to_string(static_cast<int>(pos.x)) + ", "
            + std::to_string(static_cast<int>(pos.y)) + ", "
            + std::to_string(static_cast<int>(pos.z)) + ")"
            + " | Yaw: " + std::to_string(static_cast<int>(camera.GetYaw()));
        glfwSetWindowTitle(window.GetNativeWindow(), title.c_str());

        // ---- Clear screen ----
        Renderer::Clear(BG_RED, BG_GREEN, BG_BLUE);

        // ---- Build projection matrix (uses camera's current FOV for zoom) ----
        float aspectRatio = static_cast<float>(window.GetWidth()) /
                            static_cast<float>(window.GetHeight());
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.GetFOV()), aspectRatio, NEAR_PLANE, FAR_PLANE
        );

        // ---- Activate shader and set per-frame uniforms ----
        shader.Use();
        shader.SetMat4("u_View", camera.GetViewMatrix());
        shader.SetMat4("u_Projection", projection);
        shader.SetFloat("u_Alpha", 1.0f);
        // TODO Phase 6: shader.SetVec3("u_ViewPos", camera.GetPosition());

        // ============================================================
        // Draw scene objects
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

        // Cube 4 — tall shelf (non-uniform scale)
        shader.SetMat4("u_Model", Transform::TRS({-4.0f, 1.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.5f, 3.0f, 0.4f}));
        shader.SetVec3("u_Color", glm::vec3(0.35f, 0.20f, 0.08f));
        cubeMesh->Draw();

        // Cube 5 — rotating green cube
        static float angle = 0.0f;
        angle += 30.0f * deltaTime;
        shader.SetMat4("u_Model", Transform::TRS({4.0f, 0.5f, 0.0f}, {0.0f, angle, 0.0f}));
        shader.SetVec3("u_Color", glm::vec3(0.15f, 0.55f, 0.35f));
        cubeMesh->Draw();

        // ---- Swap buffers ----
        window.SwapBuffers();

        // TODO Phase 5: Replace demo cubes with full Scene objects
    }

    // ---- Cleanup ----
    LOG_INFO("Main loop ended — cleaning up...");
    LOG_INFO("Application exited cleanly");
    return 0;
}
