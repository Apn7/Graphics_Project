// =============================================================================
// main.cpp — Entry Point for the 3D Library Simulation (Phase 6)
// =============================================================================
// Phase 6 adds texture mapping with 3 shader modes and keyboard toggles.
//
// Controls:
//   WASD       — Move
//   Q/E        — Fly up/down
//   Mouse      — Look around
//   Scroll     — Zoom (FOV)
//   Left Shift — Sprint
//   Tab        — Toggle cursor
//   F1         — Render all (default)
//   F2         — Room shell only
//   F3         — Furniture only
//   F4         — Shelves + books only
//   F5         — Toggle wireframe
//   F6         — Toggle point cloud
//   T          — Cycle texture override (per-object → flat → simple → vertex → fragment)
//   1          — Per-object texture mode (default)
//   2          — All flat color (no textures)
//   3          — All simple texture
//   4          — All vertex blend
//   5          — All fragment blend
//   ESC        — Exit
// =============================================================================

#include "core/Window.h"
#include "core/ShaderLibrary.h"
#include "core/Camera.h"
#include "core/InputHandler.h"
#include "scene/Scene.h"
#include "scene/TextureMode.h"
#include "scene/LightState.h"
#include "renderer/Renderer.h"
#include "renderer/TextureManager.h"
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
static int g_PolyMode   = 0;   // 0 = fill, 1 = wireframe, 2 = points

// Phase 6: Global texture override — controlled by T / 1-5 keys
static GlobalTextureOverride g_TexOverride = GlobalTextureOverride::NONE;

// Phase 7: Lighting state — controlled by L, N, I, O, 6, 7, 8
static LightState g_Lights;

// Phase 7: Fan toggle — controlled by G key
static bool g_FanOn = true;

// Phase 8: Bird's Eye View — controlled by B key
// When true, the normal camera matrices are replaced with a top-down
// orthographic view looking straight down at the room center.
static bool g_BirdsEye = false;

// =============================================================================
// main — Application entry point
// =============================================================================
int main() {
    LOG_INFO("=== 3D Library Simulation — Phase 7 (Phong Lighting) ===");
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

    // ---- Step 3: Load textures ----
    TextureManager::Get().LoadAll();

    // ---- Step 4: Load shaders ----
    ShaderLibrary::Get().LoadAll();
    Shader& flatShader     = ShaderLibrary::Get().GetBasic();
    Shader& simpleShader   = ShaderLibrary::Get().GetTextureSimple();
    Shader& vertexShader   = ShaderLibrary::Get().GetTextureVertex();
    Shader& fragmentShader = ShaderLibrary::Get().GetTextureFragment();

    // ---- Step 5: Build the scene (includes AssignTextures) ----
    Scene scene;
    scene.Build();

    // ---- Step 6: Create the camera ----
    Camera camera(glm::vec3(5.0f, 1.7f, 3.5f), 180.0f, 0.0f);

    // ---- Step 7: Initialize input handler ----
    InputHandler::Init(window.GetNativeWindow(), &camera);

    // ---- Step 8: Initialize timing ----
    float lastFrameTime = 0.0f;

    // ---- Print control guide to console ----
    LOG_INFO("Entering main render loop...");
    LOG_INFO("");
    LOG_INFO("========================================");
    LOG_INFO("       3D Library — Control Guide       ");
    LOG_INFO("========================================");
    LOG_INFO("");
    LOG_INFO("  MOVEMENT");
    LOG_INFO("    W / S        — Move forward / backward");
    LOG_INFO("    A / D        — Strafe left / right");
    LOG_INFO("    Q / E        — Fly up / down");
    LOG_INFO("    Left Shift   — Sprint (hold)");
    LOG_INFO("");
    LOG_INFO("  CAMERA");
    LOG_INFO("    Mouse        — Look around");
    LOG_INFO("    Scroll       — Zoom (change FOV)");
    LOG_INFO("    Tab          — Toggle mouse cursor");
    LOG_INFO("");
    LOG_INFO("  DEBUG VIEWS");
    LOG_INFO("    F1           — Show all objects (default)");
    LOG_INFO("    F2           — Room shell only (walls/floor/ceiling)");
    LOG_INFO("    F3           — Furniture only (tables + chairs)");
    LOG_INFO("    F4           — Shelves + books only");
    LOG_INFO("    F5           — Toggle wireframe mode");
    LOG_INFO("    F6           — Toggle point cloud mode");
    LOG_INFO("");
    LOG_INFO("  TEXTURE MODES");
    LOG_INFO("    1            — Per-object mode (default)");
    LOG_INFO("    2            — All textured objects: flat color");
    LOG_INFO("    3            — All textured objects: simple texture");
    LOG_INFO("    4            — All textured objects: vertex blend");
    LOG_INFO("    5            — All textured objects: fragment blend");
    LOG_INFO("    T            — Cycle through texture modes");
    LOG_INFO("");
    LOG_INFO("  LIGHTING");
    LOG_INFO("    L            — All lights ON/OFF");
    LOG_INFO("    N            — Day / Night mode toggle");
    LOG_INFO("    I            — Directional light (sun) ON/OFF");
    LOG_INFO("    O            — Point lights (pendant lamps) ON/OFF");
    LOG_INFO("    6            — Ambient component ON/OFF");
    LOG_INFO("    7            — Diffuse component ON/OFF");
    LOG_INFO("    8            — Specular component ON/OFF");
    LOG_INFO("    G            — Ceiling fan ON/OFF");
    LOG_INFO("");
    LOG_INFO("  VIEWING & INTERACTIVES");
    LOG_INFO("    B            — Bird's Eye View (top-down orthographic)");
    LOG_INFO("    H            — Toggle Door Open/Close");
    LOG_INFO("    J            — Toggle Windows Slide Up/Down");
    LOG_INFO("");
    LOG_INFO("  GENERAL");
    LOG_INFO("    ESC          — Exit application");
    LOG_INFO("");
    LOG_INFO("========================================");

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

        // ---- Update animations (fan, door, windows) ----
        // If fan is OFF, we pause its time delta, but keep animating door/windows
        scene.UpdateAnimations(g_FanOn ? deltaTime : 0.0f, deltaTime);

        // ---- Check debug mode keys (F1-F4 scene groups, F5 wireframe, F6 points) ----
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F1) == GLFW_PRESS) g_RenderMode = 0;
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F2) == GLFW_PRESS) g_RenderMode = 1;
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F3) == GLFW_PRESS) g_RenderMode = 2;
        if (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F4) == GLFW_PRESS) g_RenderMode = 3;

        // F5 = wireframe toggle, F6 = point toggle
        static bool f5Last = false, f6Last = false;
        bool f5Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F5) == GLFW_PRESS);
        bool f6Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_F6) == GLFW_PRESS);
        if (f5Now && !f5Last) g_PolyMode = (g_PolyMode == 1) ? 0 : 1;
        if (f6Now && !f6Last) g_PolyMode = (g_PolyMode == 2) ? 0 : 2;
        f5Last = f5Now;  f6Last = f6Now;

        // ---- Phase 6: Texture override toggle (T cycles, 1-5 direct set) ----
        static bool tLast = false, k1Last = false, k2Last = false;
        static bool k3Last = false, k4Last = false, k5Last = false;
        bool tNow  = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_T)  == GLFW_PRESS);
        bool k1Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_1)  == GLFW_PRESS);
        bool k2Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_2)  == GLFW_PRESS);
        bool k3Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_3)  == GLFW_PRESS);
        bool k4Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_4)  == GLFW_PRESS);
        bool k5Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_5)  == GLFW_PRESS);

        if (tNow && !tLast) {
            int cur = static_cast<int>(g_TexOverride);
            cur = (cur + 2) % 5 - 1;  // cycle: -1 → 0 → 1 → 2 → 3 → -1
            g_TexOverride = static_cast<GlobalTextureOverride>(cur);
        }
        if (k1Now && !k1Last) g_TexOverride = GlobalTextureOverride::NONE;
        if (k2Now && !k2Last) g_TexOverride = GlobalTextureOverride::ALL_FLAT;
        if (k3Now && !k3Last) g_TexOverride = GlobalTextureOverride::ALL_SIMPLE;
        if (k4Now && !k4Last) g_TexOverride = GlobalTextureOverride::ALL_VERTEX;
        if (k5Now && !k5Last) g_TexOverride = GlobalTextureOverride::ALL_FRAGMENT;
        tLast = tNow; k1Last = k1Now; k2Last = k2Now;
        k3Last = k3Now; k4Last = k4Now; k5Last = k5Now;

        // ---- Phase 7: Lighting key handlers (L, N, I, O, 6, 7, 8, G) ----
        static bool lLast=false, nLast=false, iLast=false, oLast=false;
        static bool k6Last=false, k7Last=false, k8Last=false, gLast=false;

        bool lNow  = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_L) == GLFW_PRESS);
        bool nNow  = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_N) == GLFW_PRESS);
        bool iNow  = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_I) == GLFW_PRESS);
        bool oNow  = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_O) == GLFW_PRESS);
        bool k6Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_6) == GLFW_PRESS);
        bool k7Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_7) == GLFW_PRESS);
        bool k8Now = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_8) == GLFW_PRESS);
        bool gNow  = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_G) == GLFW_PRESS);

        if (lNow  && !lLast)  {
            g_Lights.GlobalOn = !g_Lights.GlobalOn;
            LOG_INFO(std::string("Lights: ") + (g_Lights.GlobalOn ? "ON" : "OFF"));
        }
        if (nNow  && !nLast)  {
            g_Lights.NightMode = !g_Lights.NightMode;
            if (g_Lights.NightMode) {
                g_Lights.DirLightColor     = g_Lights.NightDirColor;
                g_Lights.DirLightIntensity = g_Lights.NightDirIntensity;
                g_Lights.AmbientStrength   = g_Lights.NightAmbientStr;
                LOG_INFO("Night mode ON");
            } else {
                g_Lights.DirLightColor     = g_Lights.DayDirColor;
                g_Lights.DirLightIntensity = g_Lights.DayDirIntensity;
                g_Lights.AmbientStrength   = g_Lights.DayAmbientStr;
                LOG_INFO("Day mode ON");
            }
        }
        if (iNow  && !iLast)  {
            g_Lights.DirLightOn = !g_Lights.DirLightOn;
            LOG_INFO(std::string("Directional light: ") + (g_Lights.DirLightOn ? "ON" : "OFF"));
        }
        if (oNow  && !oLast)  {
            g_Lights.PointLightsOn = !g_Lights.PointLightsOn;
            LOG_INFO(std::string("Point lights: ") + (g_Lights.PointLightsOn ? "ON" : "OFF"));
        }
        if (k6Now && !k6Last) {
            g_Lights.AmbientOn = !g_Lights.AmbientOn;
            LOG_INFO(std::string("Ambient: ") + (g_Lights.AmbientOn ? "ON" : "OFF"));
        }
        if (k7Now && !k7Last) {
            g_Lights.DiffuseOn = !g_Lights.DiffuseOn;
            LOG_INFO(std::string("Diffuse: ") + (g_Lights.DiffuseOn ? "ON" : "OFF"));
        }
        if (k8Now && !k8Last) {
            g_Lights.SpecularOn = !g_Lights.SpecularOn;
            LOG_INFO(std::string("Specular: ") + (g_Lights.SpecularOn ? "ON" : "OFF"));
        }
        if (gNow  && !gLast)  {
            g_FanOn = !g_FanOn;
            LOG_INFO(std::string("Fan: ") + (g_FanOn ? "ON" : "OFF"));
        }
        lLast=lNow; nLast=nNow; iLast=iNow; oLast=oNow;
        k6Last=k6Now; k7Last=k7Now; k8Last=k8Now; gLast=gNow;

        // ---- Phase 8: Bird's Eye View toggle (B key) ----
        static bool bLast = false;
        bool bNow = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_B) == GLFW_PRESS);
        if (bNow && !bLast) {
            g_BirdsEye = !g_BirdsEye;
            LOG_INFO(std::string("Bird's Eye View: ") + (g_BirdsEye ? "ON" : "OFF"));
        }
        // ---- Phase 8: Interactives toggles (H, J) ----
        static bool hLast = false, jLast = false;
        bool hNow = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_H) == GLFW_PRESS);
        bool jNow = (glfwGetKey(window.GetNativeWindow(), GLFW_KEY_J) == GLFW_PRESS);
        
        if (hNow && !hLast) {
            scene.ToggleDoor();
            LOG_INFO("Door toggled.");
        }
        if (jNow && !jLast) {
            scene.ToggleWindows();
            LOG_INFO("Windows toggled.");
        }
        hLast = hNow; jLast = jNow;

        // ---- Update window title ----
        float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
        glm::vec3 pos = camera.GetPosition();
        const char* polyModeStr[] = { "Fill", "Wire(F5)", "Pts(F6)" };
        const char* texModeStr[]  = { "Flat(2)", "Simple(3)", "VtxBlend(4)", "FragBlend(5)" };
        std::string texLabel = (g_TexOverride == GlobalTextureOverride::NONE)
            ? "PerObj(1)"
            : texModeStr[static_cast<int>(g_TexOverride)];

        std::string title = "3D Library | FPS: " + std::to_string(static_cast<int>(fps))
            + " | Pos: ("
            + std::to_string(static_cast<int>(pos.x)) + ", "
            + std::to_string(static_cast<int>(pos.y)) + ", "
            + std::to_string(static_cast<int>(pos.z)) + ")"
            + " | Objs: " + std::to_string(scene.GetObjectCount())
            + " | Poly: " + polyModeStr[g_PolyMode]
            + " | Tex: " + texLabel
            + " | Light: " + (g_Lights.GlobalOn ? (g_Lights.NightMode ? "Night" : "Day") : "Off");
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

        // ---- Phase 6 + 8: Set camera matrices (or Bird's Eye override) ----
        glm::mat4 activeView       = camera.GetViewMatrix();
        glm::mat4 activeProjection = projection;
        glm::vec3 activeCamPos     = camera.GetPosition();

        if (g_BirdsEye) {
            // Bird's Eye View: camera sits just BELOW the ceiling looking straight down.
            // Ceiling slab: centered at Y=6.0, thickness=0.2 → bottom face at Y=5.9.
            // Eye at Y=5.75 puts us inside the room so the ceiling is above/behind the camera.
            // Near=0.1 clips the tiny gap above camera. Far=6.5 reaches the floor at Y=0.
            static const glm::vec3 BEV_EYE    = glm::vec3(0.0f, 5.75f, 0.001f); // inside room, under ceiling
            static const glm::vec3 BEV_TARGET = glm::vec3(0.0f, 0.0f,  0.0f);   // floor center
            static const glm::vec3 BEV_UP     = glm::vec3(0.0f, 0.0f, -1.0f);   // -Z = "north" on screen

            activeView   = glm::lookAt(BEV_EYE, BEV_TARGET, BEV_UP);
            // Orthographic bounds: match full room width/depth + small margin
            float halfW  = 9.0f;   // room X half-width = 8, +1 margin
            float halfD  = 8.0f;   // room Z half-depth = 7, +1 margin
            activeProjection = glm::ortho(-halfW, halfW, -halfD, halfD, 0.1f, 6.5f);
            activeCamPos = BEV_EYE;
        }


        scene.SetCameraMatrices(activeView, activeProjection);

        // ---- Phase 7: Upload lighting uniforms to all 4 shaders ----
        Scene::SetLighting(flatShader,     g_Lights, activeCamPos);
        Scene::SetLighting(simpleShader,   g_Lights, activeCamPos);
        Scene::SetLighting(vertexShader,   g_Lights, activeCamPos);
        Scene::SetLighting(fragmentShader, g_Lights, activeCamPos);

        // ---- Render scene ----
        switch (g_RenderMode) {
            case 0:
                // Full scene with multi-shader mode
                scene.Render(flatShader, simpleShader, vertexShader, fragmentShader, g_TexOverride);
                break;
            case 1:
                // Debug: room only (flat shader)
                flatShader.Use();
                flatShader.SetMat4("u_View", camera.GetViewMatrix());
                flatShader.SetMat4("u_Projection", projection);
                flatShader.SetFloat("u_Alpha", 1.0f);
                scene.RenderGroup(flatShader, "floor");
                scene.RenderGroup(flatShader, "ceiling");
                scene.RenderGroup(flatShader, "wall");
                break;
            case 2:
                flatShader.Use();
                flatShader.SetMat4("u_View", camera.GetViewMatrix());
                flatShader.SetMat4("u_Projection", projection);
                flatShader.SetFloat("u_Alpha", 1.0f);
                scene.RenderGroup(flatShader, "table");
                scene.RenderGroup(flatShader, "chair");
                break;
            case 3:
                flatShader.Use();
                flatShader.SetMat4("u_View", camera.GetViewMatrix());
                flatShader.SetMat4("u_Projection", projection);
                flatShader.SetFloat("u_Alpha", 1.0f);
                scene.RenderGroup(flatShader, "shelf");
                scene.RenderGroup(flatShader, "book");
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
