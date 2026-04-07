// =============================================================================
// Scene.cpp — Library Scene Construction (Phase 5)
// =============================================================================
// Implements the complete library scene using transformed cubes.
// All objects use a single shared cube mesh — only the model matrix differs.
// No OpenGL calls here — only in Mesh::Draw().
//
// Room dimensions (updated):
//   Width  (X): -8 to +8   (16 units)
//   Depth  (Z): -7 to +7   (14 units)
//   Height (Y):  0 to  6.0 (6  units)
// =============================================================================

#include "scene/Scene.h"
#include "scene/LibraryColors.h"
#include "renderer/Primitives.h"
#include "renderer/TextureManager.h"
#include "utils/Transform.h"
#include "utils/Logger.h"
#include "core/Shader.h"
#include "renderer/Mesh.h"

#include <glad/glad.h>  // Phase 6: glActiveTexture, glBindTexture

#include <cmath>      // cos, sin for fan blades
#include <glm/gtc/matrix_transform.hpp>  // translate, rotate, scale for fan assembly

using namespace LibraryColors;

// =============================================================================
// Room dimension constants — single source of truth
// =============================================================================
static constexpr float ROOM_HALF_W  = 8.0f;   // X: -8 to +8
static constexpr float ROOM_HALF_D  = 7.0f;   // Z: -7 to +7
static constexpr float ROOM_HEIGHT  = 6.0f;   // Y: 0 to 6
static constexpr float WALL_THICK   = 0.2f;

// =============================================================================
// Constructor / Destructor
// =============================================================================
Scene::Scene()
    : m_CubeMesh(nullptr)
{
}

Scene::~Scene() = default;

// =============================================================================
// Add helpers
// =============================================================================
void Scene::Add(const std::string& label,
                const glm::vec3& position,
                const glm::vec3& scale,
                const glm::vec3& color)
{
    m_Objects.push_back({
        Transform::TRS(position, glm::vec3(0.0f), scale),
        Transform::TRS(position, glm::vec3(0.0f), scale), // OriginalTransform
        color,
        label
    });
}

void Scene::Add(const std::string& label,
                const glm::vec3& position,
                const glm::vec3& rotDeg,
                const glm::vec3& scale,
                const glm::vec3& color)
{
    m_Objects.push_back({
        Transform::TRS(position, rotDeg, scale),
        Transform::TRS(position, rotDeg, scale), // OriginalTransform
        color,
        label
    });
}

// =============================================================================
// Build
// =============================================================================
void Scene::Build() {
    m_CubeMesh = Primitives::CreateCube();
    m_Objects.clear();

    BuildRoom();
    BuildWindows();
    BuildDoor();
    BuildWallShelves();
    BuildBooks();
    BuildTables();
    BuildChairs();
    BuildCeiling();

    // Phase 7: pendant lamps
    BuildPendantLamps();

    // Phase 8: curved objects (sphere, cone, vase)
    BuildCurvedObjects();

    // Phase 6: assign textures and modes
    AssignTextures();

    LOG_INFO("Scene built: " + std::to_string(m_Objects.size()) + " objects");
}

// =============================================================================
// Render / RenderGroup
// =============================================================================
void Scene::Render(Shader& shader) {
    for (const auto& obj : m_Objects) {
        shader.SetMat4("u_Model", obj.Transform);
        shader.SetVec3("u_Color", obj.Color);
        m_CubeMesh->Draw();
    }
}

// =============================================================================
// Multi-shader Render (Phase 6)
// =============================================================================
void Scene::Render(Shader& flatShader,
                   Shader& simpleShader,
                   Shader& vertexBlendShader,
                   Shader& fragmentBlendShader,
                   GlobalTextureOverride override)
{
    for (const auto& obj : m_Objects) {
        // Determine effective mode
        TextureMode effectiveMode = obj.Mode;
        // Override only applies to textured objects (floor, walls, table tops)
        // Non-textured objects (chairs, shelves, books, etc.) always stay flat
        if (override != GlobalTextureOverride::NONE && obj.TextureID != 0)
            effectiveMode = static_cast<TextureMode>(static_cast<int>(override));

        // Select the correct shader
        Shader* shader = nullptr;
        switch (effectiveMode) {
            case TextureMode::SIMPLE_TEXTURE:  shader = &simpleShader;        break;
            case TextureMode::VERTEX_BLEND:    shader = &vertexBlendShader;   break;
            case TextureMode::FRAGMENT_BLEND:  shader = &fragmentBlendShader; break;
            default:                           shader = &flatShader;          break;
        }

        shader->Use();

        // Common uniforms
        shader->SetMat4("u_Model",      obj.Transform);
        shader->SetMat4("u_View",       m_View);
        shader->SetMat4("u_Projection", m_Projection);

        // Texture-specific uniforms
        if (effectiveMode != TextureMode::FLAT_COLOR) {
            shader->SetFloat("u_TileX", obj.UVTileX);
            shader->SetFloat("u_TileY", obj.UVTileY);
            shader->SetInt("u_Texture", 0);

            if (obj.TextureID != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.TextureID);
            }
        }

        // Color blend uniforms (vertex and fragment blend modes)
        if (effectiveMode == TextureMode::VERTEX_BLEND ||
            effectiveMode == TextureMode::FRAGMENT_BLEND) {
            shader->SetVec3("u_SurfaceColor", obj.Color);
            shader->SetFloat("u_BlendFactor",  obj.BlendFactor);
        }

        // Flat color fallback
        if (effectiveMode == TextureMode::FLAT_COLOR) {
            shader->SetVec3("u_Color",  obj.Color);
            shader->SetFloat("u_Alpha", 1.0f);
        }

        m_CubeMesh->Draw();
    }

    // ---- Phase 8: Render curved objects (each has its own mesh) ----
    for (const auto& obj : m_CurvedObjects) {
        TextureMode effectiveMode = obj.Mode;
        if (override != GlobalTextureOverride::NONE && obj.TextureID != 0)
            effectiveMode = static_cast<TextureMode>(static_cast<int>(override));

        Shader* shader = nullptr;
        switch (effectiveMode) {
            case TextureMode::SIMPLE_TEXTURE:  shader = &simpleShader;        break;
            case TextureMode::VERTEX_BLEND:    shader = &vertexBlendShader;   break;
            case TextureMode::FRAGMENT_BLEND:  shader = &fragmentBlendShader; break;
            default:                           shader = &flatShader;           break;
        }

        shader->Use();
        shader->SetMat4("u_Model",      obj.Transform);
        shader->SetMat4("u_View",       m_View);
        shader->SetMat4("u_Projection", m_Projection);

        if (effectiveMode != TextureMode::FLAT_COLOR) {
            shader->SetFloat("u_TileX", 1.0f);
            shader->SetFloat("u_TileY", 1.0f);
            shader->SetInt("u_Texture", 0);
            if (obj.TextureID != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.TextureID);
            }
        }
        if (effectiveMode == TextureMode::VERTEX_BLEND ||
            effectiveMode == TextureMode::FRAGMENT_BLEND) {
            shader->SetVec3("u_SurfaceColor", obj.Color);
            shader->SetFloat("u_BlendFactor",  obj.BlendFactor);
        }
        if (effectiveMode == TextureMode::FLAT_COLOR) {
            shader->SetVec3("u_Color",  obj.Color);
            shader->SetFloat("u_Alpha", 1.0f);
        }

        obj.Mesh->Draw();
    }
}

// =============================================================================
// SetCameraMatrices
// =============================================================================
void Scene::SetCameraMatrices(const glm::mat4& view, const glm::mat4& projection) {
    m_View       = view;
    m_Projection = projection;
}

// =============================================================================
// SetLighting — Upload all LightState uniforms to a shader (Phase 7)
// =============================================================================
// Call this for EVERY active shader once per frame, BEFORE Render().
// The shader must be bound (shader.Use()) or it won't receive the uniforms.
// =============================================================================
void Scene::SetLighting(Shader& shader, const LightState& lights, const glm::vec3& viewPos)
{
    shader.Use();

    // Global toggles
    shader.SetBool ("u_LightsOn",         lights.GlobalOn);
    shader.SetBool ("u_AmbientOn",        lights.AmbientOn);
    shader.SetBool ("u_DiffuseOn",        lights.DiffuseOn);
    shader.SetBool ("u_SpecularOn",       lights.SpecularOn);

    // Camera position (for specular reflection)
    shader.SetVec3 ("u_ViewPos",          viewPos);

    // Material defaults
    shader.SetFloat("u_AmbientStrength",  lights.AmbientStrength);
    shader.SetFloat("u_Shininess",        lights.Shininess);
    shader.SetFloat("u_SpecularStrength", lights.SpecularStrength);

    // Directional light
    shader.SetBool ("u_DirLightOn",        lights.DirLightOn);
    shader.SetVec3 ("u_DirLightDir",       lights.DirLightDir);
    shader.SetVec3 ("u_DirLightColor",     lights.DirLightColor);
    shader.SetFloat("u_DirLightIntensity", lights.DirLightIntensity);

    // Point lights
    shader.SetBool ("u_PointLightsOn", lights.PointLightsOn);
    for (int i = 0; i < LightState::NUM_POINT_LIGHTS; i++) {
        shader.SetVec3("u_PointLightPos["   + std::to_string(i) + "]",
                       lights.PointLights[i].Position);
        shader.SetVec3("u_PointLightColor[" + std::to_string(i) + "]",
                       lights.PointLights[i].Color);
    }
    // Attenuation coefficients (same for all point lights)
    shader.SetFloat("u_PointConstant",  lights.PointLights[0].Constant);
    shader.SetFloat("u_PointLinear",    lights.PointLights[0].Linear);
    shader.SetFloat("u_PointQuadratic", lights.PointLights[0].Quadratic);
}

void Scene::RenderGroup(Shader& shader, const std::string& groupPrefix) {
    for (const auto& obj : m_Objects) {
        if (obj.Label.rfind(groupPrefix, 0) == 0) {
            shader.SetMat4("u_Model", obj.Transform);
            shader.SetVec3("u_Color", obj.Color);
            m_CubeMesh->Draw();
        }
    }
}

// =============================================================================
// BuildRoom — Floor, ceiling, 4 walls
// =============================================================================
// Room: X[-8,+8], Z[-7,+7], Y[0,6]
//
// Side walls are built as sets of segments to leave hollow openings for doors
// and windows so the player sees through when they are opened.
//
// Opening bounds (world space, shared with BuildWindows/BuildDoor):
//   Window: Z = [-2.6, -0.4],  Y = [1.5, 4.1]  (both side walls)
//   Door:   Z = [ 2.5,  4.5],  Y = [0.0, 3.8]  (right wall only)
// =============================================================================
void Scene::BuildRoom() {
    float fullW = ROOM_HALF_W * 2.0f;  // 16
    float fullD = ROOM_HALF_D * 2.0f;  // 14
    float midY  = ROOM_HEIGHT * 0.5f;  // 3

    // Floor
    Add("floor",   {0, 0, 0},           {fullW, WALL_THICK, fullD}, FLOOR_TILE);
    // Ceiling
    Add("ceiling", {0, ROOM_HEIGHT, 0}, {fullW, WALL_THICK, fullD}, CEILING_WOOD);

    // Back wall (Z = -7) — solid, no openings
    Add("wall_back",  {0, midY, -ROOM_HALF_D}, {fullW, ROOM_HEIGHT, WALL_THICK}, WALL_CREAM);
    // Front wall (Z = +7) — solid, no openings
    Add("wall_front", {0, midY,  ROOM_HALF_D}, {fullW, ROOM_HEIGHT, WALL_THICK}, WALL_CREAM);

    // -----------------------------------------------------------------------
    // Shared opening boundaries
    // -----------------------------------------------------------------------
    constexpr float W_Z0 = -2.6f, W_Z1 = -0.4f;  // Window Z
    constexpr float W_Y0 =  1.5f, W_Y1 =  4.1f;  // Window Y
    constexpr float D_Z0 =  2.5f, D_Z1 =  4.5f;  // Door Z
    constexpr float D_Y1 =  3.8f;                  // Door top (bottom = floor)

    // Helper: add one rectangular wall slab given world-space Z and Y extents.
    // Skips degenerate slabs automatically.
    auto Slab = [&](const std::string& label, float x,
                    float z0, float z1, float y0, float y1) {
        float wz = z1 - z0;
        float wy = y1 - y0;
        if (wz <= 0.0f || wy <= 0.0f) return;
        Add(label, {x, (y0 + y1) * 0.5f, (z0 + z1) * 0.5f},
            {WALL_THICK, wy, wz}, WALL_CREAM);
    };

    // --- Left wall (X = -8): one window hole ---
    // Y band 0: [0, W_Y0] — fully solid
    Slab("wall_left_b0",    -ROOM_HALF_W, -ROOM_HALF_D, ROOM_HALF_D, 0.0f,        W_Y0);
    // Y band 1: [W_Y0, W_Y1] — skip window column
    Slab("wall_left_b1_l",  -ROOM_HALF_W, -ROOM_HALF_D, W_Z0,        W_Y0,        W_Y1);
    Slab("wall_left_b1_r",  -ROOM_HALF_W, W_Z1,         ROOM_HALF_D, W_Y0,        W_Y1);
    // Y band 2: [W_Y1, top] — fully solid
    Slab("wall_left_b2",    -ROOM_HALF_W, -ROOM_HALF_D, ROOM_HALF_D, W_Y1,        ROOM_HEIGHT);

    // --- Right wall (X = +8): window hole + door hole ---
    // Y band 0: [0, W_Y0=1.5] — only door hole
    Slab("wall_right_b0_l",  ROOM_HALF_W, -ROOM_HALF_D, D_Z0,        0.0f,        W_Y0);
    Slab("wall_right_b0_r",  ROOM_HALF_W, D_Z1,         ROOM_HALF_D, 0.0f,        W_Y0);
    // Y band 1: [W_Y0=1.5, D_Y1=3.8] — window AND door holes
    Slab("wall_right_b1_l",  ROOM_HALF_W, -ROOM_HALF_D, W_Z0,        W_Y0,        D_Y1);
    Slab("wall_right_b1_m",  ROOM_HALF_W, W_Z1,         D_Z0,        W_Y0,        D_Y1);
    Slab("wall_right_b1_r",  ROOM_HALF_W, D_Z1,         ROOM_HALF_D, W_Y0,        D_Y1);
    // Y band 2: [D_Y1=3.8, W_Y1=4.1] — only window hole (above door)
    Slab("wall_right_b2_l",  ROOM_HALF_W, -ROOM_HALF_D, W_Z0,        D_Y1,        W_Y1);
    Slab("wall_right_b2_r",  ROOM_HALF_W, W_Z1,         ROOM_HALF_D, D_Y1,        W_Y1);
    // Y band 3: [W_Y1=4.1, top] — fully solid
    Slab("wall_right_b3",    ROOM_HALF_W, -ROOM_HALF_D, ROOM_HALF_D, W_Y1,        ROOM_HEIGHT);
}

// =============================================================================
// BuildWindows — Royal Bengali double-shutter windows
// =============================================================================
// Opening bounds (must match BuildRoom constants):
//   Z = [W_Z0=-2.6, W_Z1=-0.4]  center W_ZC=-1.5
//   Y = [W_Y0=1.5,  W_Y1=4.1]   center W_YC=2.8
//
// Each window has:
//   - A fixed frame (top jamb, bottom sill, outer jambs, centre divider)
//   - LEFT  shutter: hinged at Z=W_Z0, free edge at Z=W_ZC, swings inward
//   - RIGHT shutter: hinged at Z=W_Z1, free edge at Z=W_ZC, swings inward
//   - One knob per shutter (inner + outer face) near the free edge
//
// m_WindowIndices layout (filled in order across both windows):
//   [0..3]  = panels  (LW-left, LW-right, RW-left, RW-right)
//   [4..7]  = inner knobs
//   [8..11] = outer knobs
// =============================================================================
void Scene::BuildWindows() {
    constexpr float W_Z0  = -2.6f,  W_Z1  = -0.4f,  W_ZC  = -1.5f;
    constexpr float W_Y0  =  1.5f,  W_Y1  =  4.1f,  W_YC  =  2.8f;
    constexpr float PW    =  1.1f;   // panel width  (= W_ZC - W_Z0)
    constexpr float PH    =  2.6f;   // panel height (= W_Y1 - W_Y0)
    constexpr float PT    =  0.07f;  // panel thickness (X)
    constexpr float FT    =  0.10f;  // frame strip thickness
    constexpr float FD    =  0.14f;  // frame strip depth (X)

    // Indices are pushed in four passes to maintain the documented layout.
    // We use four staging vectors then append to m_WindowIndices at the end.
    std::vector<size_t> panelIdx, knobInIdx, knobOutIdx;

    // Build one complete window on wall face wx.
    // innerSign: +1 for left wall (inner = +X), -1 for right wall (inner = -X)
    auto BuildWindow = [&](const std::string& name, float wx, float innerSign) {

        // ---- Fixed frame ------------------------------------------------
        // Top jamb
        Add(name + "_frame_top",
            {wx, W_Y1 + FT * 0.5f, W_ZC},
            {FD, FT, (W_Z1 - W_Z0) + FT * 2.0f}, WOOD_DARK);
        // Bottom sill
        Add(name + "_frame_bot",
            {wx, W_Y0 - FT * 0.5f, W_ZC},
            {FD, FT, (W_Z1 - W_Z0) + FT * 2.0f}, WOOD_DARK);
        // Low-Z jamb (hinge side of left shutter)
        Add(name + "_frame_lz",
            {wx, W_YC, W_Z0 - FT * 0.5f},
            {FD, PH, FT}, WOOD_DARK);
        // High-Z jamb (hinge side of right shutter)
        Add(name + "_frame_hz",
            {wx, W_YC, W_Z1 + FT * 0.5f},
            {FD, PH, FT}, WOOD_DARK);
        // Centre divider (where the two free edges meet)
        Add(name + "_frame_mid",
            {wx, W_YC, W_ZC},
            {FD, PH, FT * 0.7f}, WOOD_DARK);

        // ---- Left shutter (hinged at W_Z0) ------------------------------
        size_t lpIdx = m_Objects.size();
        // Outer frame of shutter
        Add(name + "_left_panel",
            {wx, W_YC, W_Z0 + PW * 0.5f},
            {PT, PH, PW}, WOOD_MEDIUM);
        // Glass fill (thinner, slightly proud on inner face)
        Add(name + "_left_glass",
            {wx + innerSign * (PT * 0.5f + 0.015f), W_YC, W_Z0 + PW * 0.5f},
            {0.025f, PH - 0.08f, PW - 0.08f}, WINDOW_TEAL);
        // Horizontal rail (mid-height decorative bar)
        Add(name + "_left_rail",
            {wx + innerSign * (PT * 0.5f + 0.018f), W_YC, W_Z0 + PW * 0.5f},
            {0.03f, FT, PW - 0.08f}, WOOD_DARK);
        panelIdx.push_back(lpIdx);

        // Inner knob (near free edge, inner face)
        size_t lkiIdx = m_Objects.size();
        Add(name + "_left_knob_in",
            {wx + innerSign * (PT * 0.5f + 0.05f), W_YC, W_ZC - 0.12f},
            {0.08f, 0.08f, 0.06f}, METAL_DARK);
        knobInIdx.push_back(lkiIdx);

        // Outer knob (outer face)
        size_t lkoIdx = m_Objects.size();
        Add(name + "_left_knob_out",
            {wx - innerSign * (PT * 0.5f + 0.05f), W_YC, W_ZC - 0.12f},
            {0.08f, 0.08f, 0.06f}, METAL_DARK);
        knobOutIdx.push_back(lkoIdx);

        // ---- Right shutter (hinged at W_Z1) -----------------------------
        size_t rpIdx = m_Objects.size();
        Add(name + "_right_panel",
            {wx, W_YC, W_Z1 - PW * 0.5f},
            {PT, PH, PW}, WOOD_MEDIUM);
        Add(name + "_right_glass",
            {wx + innerSign * (PT * 0.5f + 0.015f), W_YC, W_Z1 - PW * 0.5f},
            {0.025f, PH - 0.08f, PW - 0.08f}, WINDOW_TEAL);
        Add(name + "_right_rail",
            {wx + innerSign * (PT * 0.5f + 0.018f), W_YC, W_Z1 - PW * 0.5f},
            {0.03f, FT, PW - 0.08f}, WOOD_DARK);
        panelIdx.push_back(rpIdx);

        // Inner knob
        size_t rkiIdx = m_Objects.size();
        Add(name + "_right_knob_in",
            {wx + innerSign * (PT * 0.5f + 0.05f), W_YC, W_ZC + 0.12f},
            {0.08f, 0.08f, 0.06f}, METAL_DARK);
        knobInIdx.push_back(rkiIdx);

        // Outer knob
        size_t rkoIdx = m_Objects.size();
        Add(name + "_right_knob_out",
            {wx - innerSign * (PT * 0.5f + 0.05f), W_YC, W_ZC + 0.12f},
            {0.08f, 0.08f, 0.06f}, METAL_DARK);
        knobOutIdx.push_back(rkoIdx);
    };

    float leftX  = -ROOM_HALF_W + 0.11f;
    float rightX =  ROOM_HALF_W - 0.11f;
    BuildWindow("window_left",  leftX,  +1.0f);
    BuildWindow("window_right", rightX, -1.0f);

    // Commit indices in documented order: panels, inner knobs, outer knobs
    for (size_t i : panelIdx)   m_WindowIndices.push_back(i);
    for (size_t i : knobInIdx)  m_WindowIndices.push_back(i);
    for (size_t i : knobOutIdx) m_WindowIndices.push_back(i);
}


// =============================================================================
// BuildDoor — Wooden door on right wall
// =============================================================================
// Door opening matches the hole cut in wall_right by BuildRoom():
//   Z = [2.5, 4.5],  Y = [0, 3.8]
//
// Hinge is on the HIGH-Z side (Z=4.5). The door swings open toward lower X
// (into the room). Handle/knob is near the LOW-Z free edge (Z≈2.8),
// visible on both indoor (-X) and outdoor (+X) faces.
// =============================================================================
void Scene::BuildDoor() {
    float doorX  = ROOM_HALF_W - 0.1f;  // Inner face of right wall

    // Opening extents (must match BuildRoom constants)
    constexpr float D_Z0 = 2.5f, D_Z1 = 4.5f;
    constexpr float D_Y1 = 3.8f;
    constexpr float FT   = 0.12f;  // Frame strip thickness
    constexpr float FD   = 0.18f;  // Frame depth (X)

    float frameZCenter = (D_Z0 + D_Z1) * 0.5f;
    float frameZSpan   = (D_Z1 - D_Z0) + FT * 2.0f;

    // Top jamb
    Add("door_frame_top",
        {doorX, D_Y1 + FT * 0.5f, frameZCenter},
        {FD, FT, frameZSpan}, WOOD_DARK);

    // Latch-side jamb (low Z = free edge of door)
    Add("door_frame_latch",
        {doorX, D_Y1 * 0.5f, D_Z0 - FT * 0.5f},
        {FD, D_Y1, FT}, WOOD_DARK);

    // Hinge-side jamb (high Z = pivot side)
    Add("door_frame_hinge",
        {doorX, D_Y1 * 0.5f, D_Z1 + FT * 0.5f},
        {FD, D_Y1, FT}, WOOD_DARK);

    // Door panel — centered in opening
    size_t doorIdx = m_Objects.size();
    Add("door",
        {doorX, D_Y1 * 0.5f, (D_Z0 + D_Z1) * 0.5f},
        {0.16f, D_Y1, D_Z1 - D_Z0}, WOOD_MEDIUM);

    // Indoor handle — near the free (latch) edge, inner face
    size_t handleInIdx = m_Objects.size();
    Add("door_handle_in",
        {doorX - 0.12f, 1.8f, D_Z0 + 0.3f},
        {0.15f, 0.08f, 0.08f}, METAL_DARK);

    // Outdoor handle — same position, outer face (+X side)
    size_t handleOutIdx = m_Objects.size();
    Add("door_handle_out",
        {doorX + 0.12f, 1.8f, D_Z0 + 0.3f},
        {0.15f, 0.08f, 0.08f}, METAL_DARK);

    m_DoorIndices.push_back(doorIdx);
    m_DoorIndices.push_back(handleInIdx);
    m_DoorIndices.push_back(handleOutIdx);
}

// =============================================================================
// BuildWallShelves — Back wall (3 medium units) + Front wall (3 medium units, mirrored)
// =============================================================================
// Shelf panel centers at y=1.8 so bottom edge = 1.8 - 1.8 = 0 (floor level)
// No separate base piece — side panels start at floor, no gap.
// =============================================================================
void Scene::BuildWallShelves() {
    // ---------------------------------------------------------------
    // Medium shelf on back wall — back panel at z=-6.6, extends inward
    // Panels: center y=1.8, scale.y=3.6 → bottom=0 (floor), top=3.6
    // ---------------------------------------------------------------
    auto BuildMediumShelf = [&](float xCenter, const std::string& prefix) {
        float z   = -ROOM_HALF_D + 0.4f;  // Back panel hugging back wall
        float cy  = 1.8f;                  // Center Y: bottom = cy - 1.8 = 0
        float h   = 3.6f;                  // Total height
        float dz  = 0.35f;                 // Depth offset inward

        Add(prefix + "_back",   {xCenter, cy, z},                  {1.8f, h, 0.1f},    WOOD_DARK);
        Add(prefix + "_left",   {xCenter - 0.85f, cy, z + dz},    {0.08f, h, 0.7f},   WOOD_DARK);
        Add(prefix + "_right",  {xCenter + 0.85f, cy, z + dz},    {0.08f, h, 0.7f},   WOOD_DARK);
        Add(prefix + "_top",    {xCenter, cy + h * 0.5f - 0.04f, z + dz}, {1.8f, 0.07f, 0.7f}, WOOD_DARK);

        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.75f;
            Add(prefix + "_shelf" + std::to_string(i),
                {xCenter, shelfY, z + dz},
                {1.8f, 0.06f, 0.7f}, WOOD_DARK);
        }
    };

    BuildMediumShelf(-4.5f, "shelf_back_A");
    BuildMediumShelf( 0.0f, "shelf_back_B");
    BuildMediumShelf( 4.5f, "shelf_back_C");

    // ---------------------------------------------------------------
    // Front wall mirror — same math, depth offset goes inward (z - dz)
    // ---------------------------------------------------------------
    auto BuildMediumShelfFront = [&](float xCenter, const std::string& prefix) {
        float z   = ROOM_HALF_D - 0.4f;   // Back panel hugging front wall
        float cy  = 1.8f;
        float h   = 3.6f;
        float dz  = 0.35f;

        Add(prefix + "_back",   {xCenter, cy, z},                  {1.8f, h, 0.1f},    WOOD_DARK);
        Add(prefix + "_left",   {xCenter - 0.85f, cy, z - dz},    {0.08f, h, 0.7f},   WOOD_DARK);
        Add(prefix + "_right",  {xCenter + 0.85f, cy, z - dz},    {0.08f, h, 0.7f},   WOOD_DARK);
        Add(prefix + "_top",    {xCenter, cy + h * 0.5f - 0.04f, z - dz}, {1.8f, 0.07f, 0.7f}, WOOD_DARK);

        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.75f;
            Add(prefix + "_shelf" + std::to_string(i),
                {xCenter, shelfY, z - dz},
                {1.8f, 0.06f, 0.7f}, WOOD_DARK);
        }
    };

    BuildMediumShelfFront(-4.5f, "shelf_front_A");
    BuildMediumShelfFront( 0.0f, "shelf_front_B");
    BuildMediumShelfFront( 4.5f, "shelf_front_C");
}



// =============================================================================
// BuildBooks — Fill all shelves with colorful books
// =============================================================================
void Scene::BuildBooks() {
    const glm::vec3 bookColors[] = {
        BOOK_RED, BOOK_BLUE, BOOK_GREEN, BOOK_YELLOW,
        BOOK_ORANGE, BOOK_PURPLE, BOOK_WHITE, BOOK_BROWN
    };
    const int colorCount = 8;

    auto FillShelfRow = [&](const std::string& prefix,
                            glm::vec3 shelfPos, float shelfWidth, float shelfDepth) {
        float bookW = 0.07f;
        float bookH = 0.55f;
        float bookD = shelfDepth * 0.85f;

        int count = static_cast<int>(shelfWidth / (bookW + 0.01f));
        float startX = shelfPos.x - shelfWidth * 0.5f + bookW * 0.5f;

        for (int i = 0; i < count; i++) {
            glm::vec3 bPos = {
                startX + i * (bookW + 0.01f),
                shelfPos.y + bookH * 0.5f + 0.03f,
                shelfPos.z
            };
            Add(prefix + "_book" + std::to_string(i),
                bPos, {bookW, bookH, bookD}, bookColors[i % colorCount]);
        }
    };

    // --- Back wall medium shelves (3 units × 4 shelves each) ---
    auto FillMediumShelf = [&](float xCenter, const std::string& prefix) {
        float z = -ROOM_HALF_D + 0.75f;
        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.75f;
            FillShelfRow(prefix + "_s" + std::to_string(i),
                         {xCenter, shelfY, z}, 1.7f, 0.6f);
        }
    };

    FillMediumShelf(-4.5f, "books_back_A");
    FillMediumShelf( 0.0f, "books_back_B");
    FillMediumShelf( 4.5f, "books_back_C");

    // --- Front wall medium shelves (exact mirror of back wall) ---
    auto FillMediumShelfFront = [&](float xCenter, const std::string& prefix) {
        float z = ROOM_HALF_D - 0.75f;   // Mirrored
        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.75f;
            FillShelfRow(prefix + "_s" + std::to_string(i),
                         {xCenter, shelfY, z}, 1.7f, 0.6f);
        }
    };

    FillMediumShelfFront(-4.5f, "books_front_A");
    FillMediumShelfFront( 0.0f, "books_front_B");
    FillMediumShelfFront( 4.5f, "books_front_C");
}


// =============================================================================
// BuildTables — One table per shelf column (X = -4.5, 0, +4.5)
// =============================================================================
void Scene::BuildTables() {
    auto BuildTableSet = [&](const std::string& id, float cx) {
        // Tabletop
        Add("table_" + id + "_top", {cx, 0.78f, 0.0f}, {2.4f, 0.10f, 1.2f}, WOOD_TABLE);
        // 4 legs
        Add("table_" + id + "_leg0", {cx - 1.0f, 0.39f, -0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add("table_" + id + "_leg1", {cx + 1.0f, 0.39f, -0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add("table_" + id + "_leg2", {cx - 1.0f, 0.39f, +0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add("table_" + id + "_leg3", {cx + 1.0f, 0.39f, +0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
    };

    BuildTableSet("L",  -4.5f);   // Left   — aligned with shelf_back_A / shelf_front_A
    BuildTableSet("C",   0.0f);   // Center — aligned with shelf_back_B / shelf_front_B
    BuildTableSet("R",  +4.5f);   // Right  — aligned with shelf_back_C / shelf_front_C
}

// =============================================================================
// BuildChairs — 4 chairs per table set (2 per long Z-side), 3 sets
// =============================================================================
// Each chair at Z=±1.10 from table center, X=±0.60 from table center
// facingSign +1 = faces -Z (sits on +Z side, backrest toward +Z)
// facingSign -1 = faces +Z (sits on -Z side, backrest toward -Z)
// =============================================================================
void Scene::BuildChairs() {
    auto BuildChair = [&](const std::string& prefix, glm::vec3 pos, float facingSign) {
        // Seat
        Add(prefix + "_seat_frame", pos + glm::vec3(0, 0.48f, 0),
            {0.50f, 0.05f, 0.50f}, WOOD_MEDIUM);
        Add(prefix + "_cushion", pos + glm::vec3(0, 0.52f, 0),
            {0.44f, 0.06f, 0.44f}, CUSHION_BLUE);

        // Backrest — behind the sitter (away from table)
        glm::vec3 backPos = pos + glm::vec3(0, 0.85f, facingSign * 0.22f);
        Add(prefix + "_back_frame",   backPos,                               {0.50f, 0.60f, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_back_cushion", backPos + glm::vec3(0, 0, -facingSign * 0.03f),
            {0.44f, 0.54f, 0.04f}, CUSHION_BLUE);

        // 4 legs
        float lx = 0.20f, lz = 0.20f, legH = 0.48f;
        Add(prefix + "_leg0", pos + glm::vec3(-lx, legH * 0.5f, -lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_leg1", pos + glm::vec3( lx, legH * 0.5f, -lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_leg2", pos + glm::vec3(-lx, legH * 0.5f,  lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_leg3", pos + glm::vec3( lx, legH * 0.5f,  lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);

        // Armrests
        Add(prefix + "_arm_l", pos + glm::vec3(-0.27f, 0.65f, 0), {0.04f, 0.04f, 0.48f}, WOOD_MEDIUM);
        Add(prefix + "_arm_r", pos + glm::vec3( 0.27f, 0.65f, 0), {0.04f, 0.04f, 0.48f}, WOOD_MEDIUM);
    };

    auto BuildChairSet = [&](const std::string& id, float cx) {
        // +Z side: 2 chairs facing -Z (toward table)
        BuildChair("chair_" + id + "_zp_l", {cx - 0.60f, 0.0f,  1.10f}, +1.0f);
        BuildChair("chair_" + id + "_zp_r", {cx + 0.60f, 0.0f,  1.10f}, +1.0f);
        // -Z side: 2 chairs facing +Z (toward table)
        BuildChair("chair_" + id + "_zn_l", {cx - 0.60f, 0.0f, -1.10f}, -1.0f);
        BuildChair("chair_" + id + "_zn_r", {cx + 0.60f, 0.0f, -1.10f}, -1.0f);
    };

    BuildChairSet("L", -4.5f);   // Left column
    BuildChairSet("C",  0.0f);   // Center column
    BuildChairSet("R", +4.5f);   // Right column
}

// =============================================================================
// BuildCeiling — Single center ceiling fan with rigidly attached blades
// =============================================================================
// Each blade transform: T(fanCenter) * R(Y, angle) * T(localOffset) * S(size)
// This means each blade extends from the motor along local +X, and the Y
// rotation at the center spins the whole assembly — like a real fan.
// =============================================================================
void Scene::BuildCeiling() {
    float fanY = ROOM_HEIGHT - 0.6f;   // 5.4 — well below ceiling slab
    m_FanCenter = glm::vec3(0.0f, fanY, 0.0f);
    m_FanBladeIndices.clear();

    // Rod from ceiling to motor
    Add("fan_rod", {0.0f, ROOM_HEIGHT - 0.3f, 0.0f}, {0.06f, 0.6f, 0.06f}, METAL_DARK);

    // Motor housing
    Add("fan_motor", m_FanCenter, {0.30f, 0.22f, 0.30f}, METAL_DARK);

    // 4 blades at 90° apart — rigidly attached to motor
    for (int i = 0; i < 4; i++) {
        float angleDeg = static_cast<float>(i) * 90.0f;

        // Build transform: center → rotate → offset → scale
        // Offset = motor_half(0.15) + blade_half(0.45) = 0.60 — blade starts exactly at motor edge
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_FanCenter);
        model = glm::rotate(model, glm::radians(angleDeg), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(0.60f, -0.05f, 0.0f));      // No overlap with motor
        model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.90f, 0.03f, 0.22f));

        size_t idx = m_Objects.size();
        m_Objects.push_back({model, model, WOOD_MEDIUM, "fan_blade" + std::to_string(i)});
        m_FanBladeIndices.push_back(idx);
    }
}

// =============================================================================
// UpdateAnimations — Spin fan blades, animate interactives (door/windows)
// =============================================================================
void Scene::UpdateAnimations(float fanDeltaTime, float globalDeltaTime) {
    // ---- 1. Fan Animation (continuous) ----
    m_FanAngle += m_FanSpeed * fanDeltaTime;
    if (m_FanAngle > 360.0f) m_FanAngle -= 360.0f;

    for (int i = 0; i < 4; i++) {
        float bladeAngle = m_FanAngle + static_cast<float>(i) * 90.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_FanCenter);
        model = glm::rotate(model, glm::radians(bladeAngle), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(0.60f, -0.05f, 0.0f));
        model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.90f, 0.03f, 0.22f));

        m_Objects[m_FanBladeIndices[i]].Transform = model;
    }

    // ---- 2. Door Animation (interactive) ----
    float doorTarget = m_DoorOpen ? 80.0f : 0.0f; // degrees
    float doorSpeed = 160.0f; // degrees per second
    if (m_DoorAngle < doorTarget) {
        m_DoorAngle += doorSpeed * globalDeltaTime;
        if (m_DoorAngle > doorTarget) m_DoorAngle = doorTarget;
    } else if (m_DoorAngle > doorTarget) {
        m_DoorAngle -= doorSpeed * globalDeltaTime;
        if (m_DoorAngle < doorTarget) m_DoorAngle = doorTarget;
    }

    // Apply door transform: hinge is at HIGH-Z edge (D_Z1=4.5).
    // Negative angle swings the free end inward (toward -X, into the room).
    // Door center offset from hinge = -D_W/2 in Z (free side is at lower Z).
    // Handle offset from hinge in Z = (D_Z0+0.3) - D_Z1 = 2.8 - 4.5 = -1.7.
    if (!m_DoorIndices.empty()) {
        constexpr float doorX = ROOM_HALF_W - 0.1f;
        constexpr float D_Z1  = 4.5f;
        constexpr float D_Y1  = 3.8f;
        constexpr float D_W   = 2.0f;

        float angle = -m_DoorAngle;  // Negate: swings inward (into room)

        // 0 = Door panel
        glm::vec3 doorHinge(doorX, D_Y1 * 0.5f, D_Z1);
        glm::mat4 modelD = glm::mat4(1.0f);
        modelD = glm::translate(modelD, doorHinge);
        modelD = glm::rotate(modelD, glm::radians(angle), glm::vec3(0, 1, 0));
        modelD = glm::translate(modelD, glm::vec3(0.0f, 0.0f, -D_W * 0.5f));
        modelD = glm::scale(modelD, glm::vec3(0.16f, D_Y1, D_W));
        m_Objects[m_DoorIndices[0]].Transform = modelD;

        // 1 = Indoor handle (inner face, -X offset)
        glm::vec3 handleHinge(doorX, 1.8f, D_Z1);
        glm::mat4 modelHIn = glm::mat4(1.0f);
        modelHIn = glm::translate(modelHIn, handleHinge);
        modelHIn = glm::rotate(modelHIn, glm::radians(angle), glm::vec3(0, 1, 0));
        modelHIn = glm::translate(modelHIn, glm::vec3(-0.12f, 0.0f, -1.7f));
        modelHIn = glm::scale(modelHIn, glm::vec3(0.15f, 0.08f, 0.08f));
        m_Objects[m_DoorIndices[1]].Transform = modelHIn;

        // 2 = Outdoor handle (outer face, +X offset)
        glm::mat4 modelHOut = glm::mat4(1.0f);
        modelHOut = glm::translate(modelHOut, handleHinge);
        modelHOut = glm::rotate(modelHOut, glm::radians(angle), glm::vec3(0, 1, 0));
        modelHOut = glm::translate(modelHOut, glm::vec3(+0.12f, 0.0f, -1.7f));
        modelHOut = glm::scale(modelHOut, glm::vec3(0.15f, 0.08f, 0.08f));
        m_Objects[m_DoorIndices[2]].Transform = modelHOut;
    }

    // ---- 3. Window Animation — double-shutter swing ----
    // Each shutter hinges at its outer Z edge and swings inward (into the room).
    // Rotation axis: Y.  Angle range: 0 → 85°.
    //
    // m_WindowIndices layout:
    //   [0] LW left panel   [1] LW right panel
    //   [2] RW left panel   [3] RW right panel
    //   [4..7]  inner knobs (same order)
    //   [8..11] outer knobs (same order)
    //
    // Panel geometry constants (must match BuildWindows):
    constexpr float W_Z0 = -2.6f, W_Z1 = -0.4f;
    constexpr float W_Y0 =  1.5f, W_Y1 =  4.1f;
    constexpr float PW   =  1.1f;   // panel width
    constexpr float PH   =  W_Y1 - W_Y0;
    constexpr float PT   =  0.07f;
    constexpr float W_YC = (W_Y0 + W_Y1) * 0.5f;
    const float leftX    = -ROOM_HALF_W + 0.11f;
    const float rightX   =  ROOM_HALF_W - 0.11f;

    float winTarget = m_WindowsOpen ? 85.0f : 0.0f;
    float winSpeed  = 120.0f; // degrees per second
    if (m_WindowAngle < winTarget) {
        m_WindowAngle += winSpeed * globalDeltaTime;
        if (m_WindowAngle > winTarget) m_WindowAngle = winTarget;
    } else if (m_WindowAngle > winTarget) {
        m_WindowAngle -= winSpeed * globalDeltaTime;
        if (m_WindowAngle < winTarget) m_WindowAngle = winTarget;
    }

    if (m_WindowIndices.size() < 12) return; // guard if BuildWindows not called

    // Per-panel descriptor: { wallX, hingeZ, panelCenterOffsetZ, angleSign, innerSignX }
    // angleSign: rotation direction so that free edge swings inward.
    //   Left  wall inner = +X → left panel: +angle, right panel: -angle
    //   Right wall inner = -X → left panel: -angle, right panel: +angle
    // innerSignX: which X direction is "inner face" of the panel
    struct PanelDesc { float wx, hZ, offZ, aSign, innerX; };
    PanelDesc descs[4] = {
        { leftX,  W_Z0, +PW * 0.5f, +1.0f, +1.0f }, // LW left  shutter
        { leftX,  W_Z1, -PW * 0.5f, -1.0f, +1.0f }, // LW right shutter
        { rightX, W_Z0, +PW * 0.5f, -1.0f, -1.0f }, // RW left  shutter
        { rightX, W_Z1, -PW * 0.5f, +1.0f, -1.0f }, // RW right shutter
    };

    // Knob offsets in local panel space (from hinge, after rotation):
    //   Z offset to knob = same sign as panel free-edge offset, reduced slightly
    //   X offsets for inner/outer knobs
    for (int i = 0; i < 4; i++) {
        const auto& d = descs[i];
        float angle = d.aSign * m_WindowAngle;
        glm::vec3 hinge(d.wx, W_YC, d.hZ);

        // Panel (includes glass and rail — animate starting from panelIdx base object)
        // The panel group consists of 3 consecutive objects starting at m_WindowIndices[i].
        // Animate each member of the group with the same hinge rotation.
        // Object local offsets from hinge (in closed position):
        //   panel:  (0, 0, offZ)
        //   glass:  (innerX*(PT/2+0.015), 0, offZ)
        //   rail:   (innerX*(PT/2+0.018), 0, offZ)
        struct GroupMember { float lx, lz; glm::vec3 scale; glm::vec3 color; };
        GroupMember members[3] = {
            { 0.0f,                        d.offZ, {PT,     PH,          PW},          WOOD_MEDIUM  },
            { d.innerX * (PT*0.5f+0.015f), d.offZ, {0.025f, PH - 0.08f, PW - 0.08f}, WINDOW_TEAL  },
            { d.innerX * (PT*0.5f+0.018f), d.offZ, {0.03f,  0.10f,      PW - 0.08f}, WOOD_DARK    },
        };

        for (int m = 0; m < 3; m++) {
            glm::mat4 mdl = glm::mat4(1.0f);
            mdl = glm::translate(mdl, hinge);
            mdl = glm::rotate(mdl, glm::radians(angle), glm::vec3(0, 1, 0));
            mdl = glm::translate(mdl, glm::vec3(members[m].lx, 0.0f, members[m].lz));
            mdl = glm::scale(mdl, members[m].scale);
            m_Objects[m_WindowIndices[i] + static_cast<size_t>(m)].Transform = mdl;
        }

        // Knob Z offset from hinge (near free edge = offZ * 2 - small margin)
        float kzOff = d.offZ * (1.8f / PW); // ~0.88 * sign of offZ, near the free edge

        // Inner knob [4..7]
        glm::mat4 mkIn = glm::mat4(1.0f);
        mkIn = glm::translate(mkIn, hinge);
        mkIn = glm::rotate(mkIn, glm::radians(angle), glm::vec3(0, 1, 0));
        mkIn = glm::translate(mkIn, glm::vec3(d.innerX * (PT * 0.5f + 0.05f), 0.0f, kzOff));
        mkIn = glm::scale(mkIn, glm::vec3(0.08f, 0.08f, 0.06f));
        m_Objects[m_WindowIndices[4 + i]].Transform = mkIn;

        // Outer knob [8..11]
        glm::mat4 mkOut = glm::mat4(1.0f);
        mkOut = glm::translate(mkOut, hinge);
        mkOut = glm::rotate(mkOut, glm::radians(angle), glm::vec3(0, 1, 0));
        mkOut = glm::translate(mkOut, glm::vec3(-d.innerX * (PT * 0.5f + 0.05f), 0.0f, kzOff));
        mkOut = glm::scale(mkOut, glm::vec3(0.08f, 0.08f, 0.06f));
        m_Objects[m_WindowIndices[8 + i]].Transform = mkOut;
    }
}

// =============================================================================
// AssignTextures — Phase 6: Set texture IDs and modes on built objects
// =============================================================================
void Scene::AssignTextures() {
    unsigned int floorTex = TextureManager::Get().GetID("floor");
    unsigned int wallTex  = TextureManager::Get().GetID("wall");
    unsigned int woodTex  = TextureManager::Get().GetID("wood");

    for (auto& obj : m_Objects) {

        // FLOOR — Simple texture, no color blend
        if (obj.Label == "floor") {
            obj.TextureID   = floorTex;
            obj.Mode        = TextureMode::SIMPLE_TEXTURE;
            obj.UVTileX     = 8.0f;   // Tile across 16-unit floor width
            obj.UVTileY     = 7.0f;   // Tile across 14-unit floor depth
        }

        // WALLS — Vertex color blend (cream tint over plaster texture)
        // UV tiling is scaled proportionally to each segment's world-space size
        // so the texture density stays uniform across all wall pieces.
        else if (obj.Label.rfind("wall_", 0) == 0) {
            obj.TextureID   = wallTex;
            obj.Mode        = TextureMode::VERTEX_BLEND;
            obj.Color       = LibraryColors::WALL_CREAM;
            obj.BlendFactor = 0.35f;

            // Extract scale from transform columns (length of each basis vector)
            float scaleX = glm::length(glm::vec3(obj.Transform[0]));
            float scaleY = glm::length(glm::vec3(obj.Transform[1]));
            float scaleZ = glm::length(glm::vec3(obj.Transform[2]));

            // Back/front walls are thin in Z, wide in X → use X as horizontal extent.
            // Left/right walls are thin in X, wide in Z → use Z as horizontal extent.
            bool isBackFront = (obj.Label == "wall_back" || obj.Label == "wall_front");
            float hExtent = isBackFront ? scaleX : scaleZ;
            float hBase   = isBackFront ? 16.0f  : 14.0f;  // Full wall span in that direction

            obj.UVTileX = (3.0f / hBase) * hExtent;   // Maintain same tile density as original
            obj.UVTileY = (2.0f / 6.0f)  * scaleY;
        }

        // TABLE TOPS — Fragment color blend (dark wood texture + dark tint)
        else if (obj.Label.find("table") != std::string::npos &&
                 obj.Label.find("top")   != std::string::npos) {
            obj.TextureID   = woodTex;
            obj.Mode        = TextureMode::FRAGMENT_BLEND;
            obj.UVTileX     = 2.0f;
            obj.UVTileY     = 1.0f;
            obj.Color       = LibraryColors::WOOD_TABLE;
            obj.BlendFactor = 0.40f;
        }

        // CEILING — Vertex blend (warm tint over ceiling texture)
        else if (obj.Label == "ceiling") {
            obj.TextureID   = TextureManager::Get().GetID("ceiling");
            obj.Mode        = TextureMode::VERTEX_BLEND;
            obj.UVTileX     = 4.0f;   // Tile 4× across 16-unit width
            obj.UVTileY     = 3.5f;   // Tile 3.5× across 14-unit depth
            obj.Color       = LibraryColors::CEILING_WOOD;
            obj.BlendFactor = 0.30f;  // Subtle tint — mostly texture
        }

        // Everything else remains FLAT_COLOR (Phase 5 behavior unchanged)
    }

    LOG_INFO("AssignTextures: Texture modes assigned to scene objects.");
}

// =============================================================================
// BuildPendantLamps — 3 hanging pendant lights above tables (Phase 7)
// =============================================================================
// Each lamp has:
//   - A thin cord from ceiling to shade
//   - A lamp shade (tapered-look cuboid)
//   - A small bright bulb cube (emissive look)
// X positions match the 3 table columns: -4.5, 0, +4.5
// Point light sources (in LightState) are positioned at Y=4.8 matching bulb pos.
// =============================================================================
void Scene::BuildPendantLamps() {
    auto BuildLamp = [&](const std::string& id, float cx) {
        // Cord — thin vertical rod from ceiling to shade
        Add("lamp_" + id + "_cord",
            {cx, 5.3f, 0.0f},
            {0.03f, 0.4f, 0.03f},
            glm::vec3(0.15f, 0.12f, 0.10f));   // Dark brown

        // Shade — wide flat box representing lampshade
        Add("lamp_" + id + "_shade",
            {cx, 4.95f, 0.0f},
            {0.40f, 0.18f, 0.40f},
            glm::vec3(0.55f, 0.45f, 0.25f));   // Warm tan/brass

        // Bulb — tiny bright cube (warm yellow glow appearance)
        Add("lamp_" + id + "_bulb",
            {cx, 4.88f, 0.0f},
            {0.10f, 0.08f, 0.10f},
            glm::vec3(1.0f, 0.95f, 0.70f));    // Warm bright yellow
    };

    BuildLamp("L", -4.5f);
    BuildLamp("C",  0.0f);
    BuildLamp("R", +4.5f);

    LOG_INFO("BuildPendantLamps: 3 pendant lamps added above tables.");
}

// =============================================================================
// BuildCurvedObjects — Globe (sphere), Lampshade (cone), Vase (Bezier) Phase 8
// =============================================================================
// Each object creates its own unique mesh via Primitives factory functions.
// Stored in m_CurvedObjects and rendered separately from the cube-based objects.
//
// Positions:
//   Globe  — sitting on the center reading table (X=0, table top Y=1.35)
//   Cones  — replace the 3 flat box lampshades (same positions as prior shades)
//   Vase   — tall decorative vase by the door (X=+7.0, Z=+3.5, floor level)
//
// Fractal plant scalability:
//   The vase is scaled to width ~0.5, height ~1.2 units.
//   A fractal tree placed at the vase rim (Y ~= vaseBase + 1.2) will look
//   like it's growing out of the vase naturally.
// =============================================================================
void Scene::BuildCurvedObjects() {
    using namespace glm;

    // ---- 1. GLOBE (textured sphere) on the center reading table ----
    // Table top is at Y=1.35. Globe radius = 0.28 (visually comfortable on table).
    // Position: center table (X=0, Z=0), sitting on top.
    {
        CurvedObject globe;
        globe.Label     = "globe";
        globe.Color     = vec3(0.3f, 0.5f, 0.8f);  // Ocean blue fallback color
        globe.Mode      = TextureMode::FLAT_COLOR;   // Will be SIMPLE_TEXTURE once user provides world map
        globe.TextureID = 0;
        // Sphere radius=1 in mesh space. Scale by 0.28 on all axes.
        // Center sits on table: Y = tableTopY + radius = 1.35 + 0.28 = 1.63
        globe.Transform = translate(mat4(1.0f), vec3(0.0f, 1.63f, 0.0f))
                        * scale(mat4(1.0f),     vec3(0.28f, 0.28f, 0.28f));
        globe.Mesh      = Primitives::CreateSphere(40, 40);
        m_CurvedObjects.push_back(std::move(globe));
    }

    // ---- 2. GLOBE STAND — small tilted axis rod (flat cube, just cosmetic) ----
    // A tiny dark rod to suggest the globe is on a stand.
    Add("globe_stand_base", {0.0f, 1.37f, 0.0f},
        {0.12f, 0.04f, 0.12f}, vec3(0.25f, 0.20f, 0.15f));
    Add("globe_stand_pole", {0.0f, 1.50f, 0.0f},
        {0.02f, 0.26f, 0.02f}, vec3(0.35f, 0.30f, 0.25f));

    // ---- 3. PENDANT LAMP CONES (replace 3 flat box lampshades) ----
    // The old shade was: {cx, 4.95f, 0.0f}, scale {0.40f, 0.18f, 0.40f}
    // The cone mesh: apex at Y=1, base at Y=0, radius=1. We want:
    //   - Apex pointing UP, base opening downward (flip with Y scale negative or rotate 180°)
    //   - Diameter ~0.50 at opening, height ~0.22
    //   - Position apex at Y=5.06 (cord attachment), base opening at Y=4.84
    // Strategy: rotate 180° around X to flip apex down→up, scale(0.25, 0.22, 0.25)
    auto AddLampCone = [&](const std::string& id, float cx) {
        CurvedObject cone;
        cone.Label   = "lamp_" + id + "_cone";
        cone.Color   = vec3(0.55f, 0.45f, 0.25f);  // Warm tan/brass
        cone.Mode    = TextureMode::FLAT_COLOR;
        // Cone: apex at Y=1 (top), base at Y=0 (bottom) in mesh space.
        // We DON'T flip — open end is already at Y=0 facing down. Perfect for a pendant.
        // Translate so cone base (Y=0 in mesh) sits at Y=4.84, apex at Y=4.84+0.22=5.06.
        cone.Transform = translate(mat4(1.0f), vec3(cx, 4.84f, 0.0f))
                       * scale(mat4(1.0f), vec3(0.25f, 0.22f, 0.25f));
        cone.Mesh    = Primitives::CreateCone(32);
        m_CurvedObjects.push_back(std::move(cone));
    };
    AddLampCone("L", -4.5f);
    AddLampCone("C",  0.0f);
    AddLampCone("R", +4.5f);

    // ---- 4. DECORATIVE BEZIER VASE by the door ----
    // Door is on right wall (X=+7.9), door center Z=3.5.
    // Vase placed inside room slightly left of door: X=+6.2, Z=+3.5, base at floor (Y=0).
    // Vase profile height ~1.18 units in mesh space. Scale to look tall: height=1.5, width=0.5.
    // Scalable: fractal tree can be placed at Y=1.5 above vase base.
    {
        CurvedObject vase;
        vase.Label   = "vase_door";
        vase.Color   = vec3(0.45f, 0.35f, 0.65f);  // Deep ceramic purple
        vase.Mode    = TextureMode::FLAT_COLOR;
        // Translate base of vase to floor level. Profile starts at Y=0 in mesh space.
        // Scale: width=0.5 (radius up to 0.5*0.5=0.25 world units), height=1.5
        vase.Transform = translate(mat4(1.0f), vec3(6.2f, 0.0f, 3.5f))
                       * scale(mat4(1.0f), vec3(0.5f, 1.5f, 0.5f));
        vase.Mesh    = Primitives::CreateBezierVase(50, 60);
        m_CurvedObjects.push_back(std::move(vase));
    }

    LOG_INFO("BuildCurvedObjects: globe, 3 lamp cones, vase created.");
}

