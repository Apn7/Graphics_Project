// =============================================================================
// main.cpp — Entry Point for the 3D Library Simulation (Phase 5)
// =============================================================================
// Demonstrates the full library scene:
//   1. Scene::Build() constructs all walls, shelves, books, tables, chairs, fans
//   2. Scene::Render() draws everything with a single cube mesh
//   3. FPS camera from Phase 4 for navigation
//   4. F1-F4 debug render modes to isolate object groups
//
// Controls:
//   WASD       — Move
//   Q/E        — Fly up/down
//   Mouse      — Look around
//   Scroll     — Zoom (FOV)
//   Left Shift — Sprint
//   Tab        — Toggle cursor
//   F1         — Render all (default)
//   F2         — Room shell only (walls/floor/ceiling)
//   F3         — Furniture only (table + chair)
//   F4         — Shelves + books only (shelf + book + island)
//   ESC        — Exit
// =============================================================================

#include "core/Window.h"
#include "core/ShaderLibrary.h"
#include "core/Camera.h"
#include "core/InputHandler.h"
#include "scene/Scene.h"
#include "renderer/Renderer.h"
#include "utils/Logger.h"

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

// Debug render mode — controlled by F1-F4 keys
static int g_RenderMode = 0;   // 0 = all, 1 = room, 2 = furniture, 3 = shelves+books

// Polygon debug mode — controlled by F5/F6 keys
// 0 = fill (normal), 1 = wireframe, 2 = points
static int g_PolyMode   = 0;

// =============================================================================
// main — Application entry point
// =============================================================================
int main() {
    LOG_INFO("=== 3D Library Simulation — Phase 5 ===");
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

    // ---- Step 3: Load shaders ----
    ShaderLibrary::Get().LoadAll();
    Shader& shader = ShaderLibrary::Get().GetBasic();

    // ---- Step 4: Build the scene ----
    Scene scene;
    scene.Build();

    // ---- Step 5: Create the camera ----
    Camera camera(glm::vec3(5.0f, 1.7f, 3.5f), 180.0f, 0.0f);

    // ---- Step 6: Initialize input handler ----
    InputHandler::Init(window.GetNativeWindow(), &camera);

    // ---- Step 7: Initialize timing ----
    float lastFrameTime = 0.0f;

    LOG_INFO("Entering main render loop...");
    LOG_INFO("Controls: WASD=move, QE=up/down, Mouse=look, Scroll=zoom, Shift=sprint, Tab=cursor, F1-F4=debug, ESC=exit");

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

        // ---- Process input ----
        InputHandler::ProcessContinuousInput(window.GetNativeWindow(), &camera, deltaTime);

        // ---- Update animations (fan rotation) ----
        scene.UpdateAnimations(deltaTime);

        // ---- Check debug mode keys (F1-F4 scene groups, F5 wireframe, F6 points) ----
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F1) == GLFW_PRESS) g_RenderMode = 0;
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F2) == GLFW_PRESS) g_RenderMode = 1;
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F3) == GLFW_PRESS) g_RenderMode = 2;
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F4) == GLFW_PRESS) g_RenderMode = 3;

        // F5 = wireframe toggle, F6 = point toggle (same key again = back to fill)
        static bool f5Last = false, f6Last = false;
        bool f5Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F5) == GLFW_PRESS);
        bool f6Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F6) == GLFW_PRESS);
        if (f5Now && !f5Last) g_PolyMode = (g_PolyMode == 1) ? 0 : 1;  // toggle wireframe
        if (f6Now && !f6Last) g_PolyMode = (g_PolyMode == 2) ? 0 : 2;  // toggle points
        f5Last = f5Now;  f6Last = f6Now;

        // ---- Update window title ----
        float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
        glm::vec3 pos = camera.GetPosition();
        const char* polyModeStr[] = { "Fill", "Wireframe(F5)", "Points(F6)" };
        std::string title = "3D Library | FPS: " + std::to_string(static_cast<int>(fps))
            + " | Pos: ("
            + std::to_string(static_cast<int>(pos.x)) + ", "
            + std::to_string(static_cast<int>(pos.y)) + ", "
            + std::to_string(static_cast<int>(pos.z)) + ")"
            + " | Yaw: " + std::to_string(static_cast<int>(camera.GetYaw()))
            + " | Objects: " + std::to_string(scene.GetObjectCount())
            + " | Draw: " + polyModeStr[g_PolyMode];
        glfwSetWindowTitle(window.GetNativeWindow(), title.c_str());

        // ---- Clear screen ----
        Renderer::Clear(BG_RED, BG_GREEN, BG_BLUE);

        // ---- Build projection matrix ----
        float aspectRatio = static_cast<float>(window.GetWidth()) /
                            static_cast<float>(window.GetHeight());
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.GetFOV()), aspectRatio, NEAR_PLANE, FAR_PLANE
        );

        // ---- Apply polygon mode for debug ----
        if (g_PolyMode == 1) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else if (g_PolyMode == 2) {
            glPointSize(4.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // ---- Activate shader and set per-frame uniforms ----
        shader.Use();
        shader.SetMat4("u_View", camera.GetViewMatrix());
        shader.SetMat4("u_Projection", projection);
        shader.SetFloat("u_Alpha", 1.0f);
        // TODO Phase 6: shader.SetVec3("u_ViewPos", camera.GetPosition());

        // ---- Render scene (or debug group) ----
        switch (g_RenderMode) {
            case 0:
                scene.Render(shader);
                break;
            case 1:
                scene.RenderGroup(shader, "floor");
                scene.RenderGroup(shader, "ceiling");
                scene.RenderGroup(shader, "wall");
                break;
            case 2:
                scene.RenderGroup(shader, "table");
                scene.RenderGroup(shader, "chair");
                break;
            case 3:
                scene.RenderGroup(shader, "shelf");
                scene.RenderGroup(shader, "book");
                scene.RenderGroup(shader, "island");
                break;
        }

        // ---- Reset polygon mode to fill for next frame ----
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // ---- Swap buffers ----
        window.SwapBuffers();
    }

    // ---- Cleanup ----
    LOG_INFO("Main loop ended — cleaning up...");
    LOG_INFO("Application exited cleanly");
    return 0;
}
