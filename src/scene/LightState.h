// =============================================================================
// LightState.h — Lighting Parameters & Toggle Flags (Phase 7)
// =============================================================================
// Single source of truth for all lighting state. Passed to Scene::SetLighting()
// each frame and uploaded as uniforms to all active shaders.
//
// Light positions use the library coordinate system:
//   X: -8 to +8   Z: -7 to +7   Y: 0 to 6
// Pendant lights hang at Y=4.8 (just below ceiling rod layer).
// =============================================================================
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct PointLight {
    glm::vec3 Position;
    glm::vec3 Color;
    float Constant  = 1.0f;
    float Linear    = 0.18f;
    float Quadratic = 0.07f;
};

struct LightState {
    // ---- Global master switch ----
    bool GlobalOn    = true;   // L key — kills all lighting when false

    // ---- Per-component toggles ----
    bool AmbientOn   = true;   // 6 key
    bool DiffuseOn   = true;   // 7 key
    bool SpecularOn  = true;   // 8 key

    // ---- Directional light (sun) ----
    bool DirLightOn              = true;   // I key
    glm::vec3 DirLightDir        = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f)); // Shines down-and-forward
    glm::vec3 DirLightColor      = glm::vec3(1.0f, 0.95f, 0.85f);  // Warm daylight
    float     DirLightIntensity  = 0.55f;

    // ---- Night mode ----
    bool NightMode = false;    // N key — switches dir light to cool moonlight

    // Night preset values (applied when NightMode = true)
    glm::vec3 NightDirColor     = glm::vec3(0.15f, 0.18f, 0.35f);  // Cool blue-grey moon
    float     NightDirIntensity = 0.15f;
    float     NightAmbientStr   = 0.04f;  // Very dark ambient at night

    // Day preset values (restored when NightMode = false)
    glm::vec3 DayDirColor       = glm::vec3(1.0f, 0.95f, 0.85f);
    float     DayDirIntensity   = 0.55f;
    float     DayAmbientStr     = 0.18f;

    // ---- Point lights (6 pendant lamps at FLFLF L-column positions) ----
    bool PointLightsOn = true;  // O key

    // L positions: X = {-4, +4}, Z = {-6, 0, +6}  → 6 lights total
    static constexpr int NUM_POINT_LIGHTS = 6;
    PointLight PointLights[NUM_POINT_LIGHTS] = {
        { glm::vec3(-4.0f, 4.8f, -6.0f), glm::vec3(1.0f, 0.88f, 0.65f) },  // Row 0 Left
        { glm::vec3(+4.0f, 4.8f, -6.0f), glm::vec3(1.0f, 0.88f, 0.65f) },  // Row 0 Right
        { glm::vec3(-4.0f, 4.8f,  0.0f), glm::vec3(1.0f, 0.88f, 0.65f) },  // Row 1 Left
        { glm::vec3(+4.0f, 4.8f,  0.0f), glm::vec3(1.0f, 0.88f, 0.65f) },  // Row 1 Right
        { glm::vec3(-4.0f, 4.8f, +6.0f), glm::vec3(1.0f, 0.88f, 0.65f) },  // Row 2 Left
        { glm::vec3(+4.0f, 4.8f, +6.0f), glm::vec3(1.0f, 0.88f, 0.65f) },  // Row 2 Right
    };

    // ---- Spot Light (librarian's study lamp on the desk) ----
    // Toggle: Key 3.  Cutoff angles stored as cosines for efficient dot-product compare.
    //   SpotCutoff      = cos(20°) ≈ 0.9397  — inner bright cone
    //   SpotOuterCutoff = cos(28°) ≈ 0.8829  — outer edge (smooth penumbra)
    bool      SpotLightOn      = true;
    glm::vec3 SpotLightPos     = glm::vec3(8.80f, 1.63f, 5.50f);  // Inside lamp head
    glm::vec3 SpotLightDir     = glm::vec3(0.0f, -1.0f, 0.0f);    // Pointing straight down
    glm::vec3 SpotLightColor   = glm::vec3(1.0f, 0.92f, 0.75f);   // Warm incandescent
    float     SpotCutoff       = 0.9397f;   // cos(20°)
    float     SpotOuterCutoff  = 0.8829f;   // cos(28°)
    float     SpotConstant     = 1.0f;
    float     SpotLinear       = 0.22f;
    float     SpotQuadratic    = 0.20f;

    // ---- Material defaults (applied to all objects uniformly) ----
    float AmbientStrength   = 0.18f;
    float Shininess         = 32.0f;
    float SpecularStrength  = 0.4f;
};
