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
static constexpr float ROOM_HALF_W  = 12.0f;  // X: -12 to +12 (width=24)
static constexpr float ROOM_HALF_D  = 10.0f;  // Z: -10 to +10 (depth=20)
static constexpr float ROOM_HEIGHT  = 6.0f;   // Y: 0 to 6
static constexpr float WALL_THICK   = 0.2f;
static const glm::vec3 LIBRARIAN_DESK_CENTER(7.5f, 0.0f, 6.0f);

static glm::mat4 RotateAroundPivotY(const glm::vec3& pivot, float degrees)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pivot);
    transform = glm::rotate(transform, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::translate(transform, -pivot);
    return transform;
}

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

    // Phase 9: fractal plant growing from the vase
    BuildFractalTree();

    // Phase 10: Librarian desk (posh desk + exec chair + study lamp)
    BuildLibrarianDesk();

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

    // Spot light (study lamp on librarian desk)
    shader.SetBool ("u_SpotLightOn",      lights.SpotLightOn);
    shader.SetVec3 ("u_SpotLightPos",     lights.SpotLightPos);
    shader.SetVec3 ("u_SpotLightDir",     lights.SpotLightDir);
    shader.SetVec3 ("u_SpotLightColor",   lights.SpotLightColor);
    shader.SetFloat("u_SpotCutoff",       lights.SpotCutoff);
    shader.SetFloat("u_SpotOuterCutoff",  lights.SpotOuterCutoff);
    shader.SetFloat("u_SpotConstant",     lights.SpotConstant);
    shader.SetFloat("u_SpotLinear",       lights.SpotLinear);
    shader.SetFloat("u_SpotQuadratic",    lights.SpotQuadratic);
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

    BuildMediumShelf(-7.5f, "shelf_back_A");
    BuildMediumShelf( 0.0f, "shelf_back_B");
    BuildMediumShelf( 7.5f, "shelf_back_C");

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

    BuildMediumShelfFront(-7.5f, "shelf_front_A");
    BuildMediumShelfFront( 0.0f, "shelf_front_B");
    // Front-right replaced by librarian desk
    // BuildMediumShelfFront( 7.5f, "shelf_front_C");
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

    FillMediumShelf(-7.5f, "books_back_A");
    FillMediumShelf( 0.0f, "books_back_B");
    FillMediumShelf( 7.5f, "books_back_C");

    // --- Front wall medium shelves (exact mirror of back wall) ---
    auto FillMediumShelfFront = [&](float xCenter, const std::string& prefix) {
        float z = ROOM_HALF_D - 0.75f;   // Mirrored
        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.75f;
            FillShelfRow(prefix + "_s" + std::to_string(i),
                         {xCenter, shelfY, z}, 1.7f, 0.6f);
        }
    };

    FillMediumShelfFront(-7.5f, "books_front_A");
    FillMediumShelfFront( 0.0f, "books_front_B");
    // Front-right replaced by librarian desk
    // FillMediumShelfFront( 7.5f, "books_front_C");
}


// =============================================================================
// BuildTables — 3x3 grid of tables under each ceiling fan (X=-7.5/0/+7.5, Z=-6/0/+6)
// =============================================================================
void Scene::BuildTables() {
    auto BuildTableSet = [&](const std::string& id, float cx, float cz) {
        // Tabletop
        Add("table_" + id + "_top", {cx, 0.78f, cz}, {2.4f, 0.10f, 1.2f}, WOOD_TABLE);
        // 4 legs
        Add("table_" + id + "_leg0", {cx - 1.0f, 0.39f, cz - 0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add("table_" + id + "_leg1", {cx + 1.0f, 0.39f, cz - 0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add("table_" + id + "_leg2", {cx - 1.0f, 0.39f, cz + 0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add("table_" + id + "_leg3", {cx + 1.0f, 0.39f, cz + 0.45f}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
    };

    // Middle row (Z=0) — under fans at Z=0
    BuildTableSet("L",   -7.5f,  0.0f);
    BuildTableSet("C",    0.0f,  0.0f);
    BuildTableSet("R",   +7.5f,  0.0f);

    // Back row (Z=-6) — under fans at Z=-6
    BuildTableSet("Lb",  -7.5f, -6.0f);
    BuildTableSet("Cb",   0.0f, -6.0f);
    BuildTableSet("Rb",  +7.5f, -6.0f);

    // Front row (Z=+6) — under fans at Z=+6
    BuildTableSet("Lf",  -7.5f, +6.0f);
    BuildTableSet("Cf",   0.0f, +6.0f);
    // BuildTableSet("Rf",  +7.5f, +6.0f); // Replaced by Librarian Desk
}

// =============================================================================
// BuildChairs — 4 chairs per table set (2 per long Z-side), 9 sets (3x3 grid)
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

    auto BuildChairSet = [&](const std::string& id, float cx, float cz) {
        // +Z side: 2 chairs facing -Z (toward table)
        BuildChair("chair_" + id + "_zp_l", {cx - 0.60f, 0.0f, cz + 1.10f}, +1.0f);
        BuildChair("chair_" + id + "_zp_r", {cx + 0.60f, 0.0f, cz + 1.10f}, +1.0f);
        // -Z side: 2 chairs facing +Z (toward table)
        BuildChair("chair_" + id + "_zn_l", {cx - 0.60f, 0.0f, cz - 1.10f}, -1.0f);
        BuildChair("chair_" + id + "_zn_r", {cx + 0.60f, 0.0f, cz - 1.10f}, -1.0f);
    };

    // Middle row (Z=0) — existing
    BuildChairSet("L",   -7.5f,  0.0f);
    BuildChairSet("C",    0.0f,  0.0f);
    BuildChairSet("R",   +7.5f,  0.0f);

    // Back row (Z=-6) — under fans at Z=-6
    BuildChairSet("Lb",  -7.5f, -6.0f);
    BuildChairSet("Cb",   0.0f, -6.0f);
    BuildChairSet("Rb",  +7.5f, -6.0f);

    // Front row (Z=+6) — under fans at Z=+6
    BuildChairSet("Lf",  -7.5f, +6.0f);
    BuildChairSet("Cf",   0.0f, +6.0f);
    // BuildChairSet("Rf",  +7.5f, +6.0f); // Replaced by Librarian Desk
}

// =============================================================================
// BuildCeiling — 9 ceiling fans + 6 pendant lamps in FLFLF grid (3 rows x 5 cols)
// =============================================================================
// Room: X in [-12,+12] (24 wide), Z in [-10,+10] (20 deep)
// FLFLF column pattern:
//   Col 0 (F): X = -8.0   Col 1 (L): X = -4.0
//   Col 2 (F): X =  0.0   Col 3 (L): X = +4.0
//   Col 4 (F): X = +8.0
// Row pattern (3 rows):
//   Row 0: Z = -6.0    Row 1: Z = 0.0    Row 2: Z = +6.0
// Fans at F columns (X=-8, 0, +8), 9 total.
// Lights at L columns (X=-4, +4), 6 total (handled in BuildPendantLamps).
// =============================================================================
void Scene::BuildCeiling() {
    const float fanY = ROOM_HEIGHT - 0.6f;   // 5.4 — below ceiling slab
    m_FanCenters.clear();
    m_FanBladeIndices.clear();

    // Fan X columns and Z rows
    const float fanX[] = { -8.0f, 0.0f, +8.0f };
    const float fanZ[] = { -6.0f, 0.0f, +6.0f };

    // Helper to add one complete fan at a given (cx, cz)
    auto AddFan = [&](float cx, float cz, int fanIdx) {
        std::string id = std::to_string(fanIdx);
        glm::vec3 center = { cx, fanY, cz };
        m_FanCenters.push_back(center);

        // Drop rod from ceiling to motor
        CurvedObject rod;
        rod.Label = "fan" + id + "_rod";
        rod.Color = METAL_DARK;
        rod.Mode = TextureMode::FLAT_COLOR;
        rod.Transform = Transform::TRS({ cx, ROOM_HEIGHT - 0.3f, cz }, glm::vec3(0.0f), glm::vec3(0.06f, 0.6f, 0.06f));
        rod.Mesh = Primitives::CreateCylinder();
        m_CurvedObjects.push_back(std::move(rod));

        // Motor housing
        CurvedObject motor;
        motor.Label = "fan" + id + "_motor";
        motor.Color = METAL_DARK;
        motor.Mode = TextureMode::FLAT_COLOR;
        motor.Transform = Transform::TRS(center, glm::vec3(0.0f), glm::vec3(0.30f, 0.22f, 0.30f));
        motor.Mesh = Primitives::CreateCylinder();
        m_CurvedObjects.push_back(std::move(motor));

        // 4 blades at 90° apart
        for (int b = 0; b < 4; b++) {
            float angleDeg = static_cast<float>(b) * 90.0f;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, center);
            model = glm::rotate(model, glm::radians(angleDeg), glm::vec3(0, 1, 0));
            model = glm::translate(model, glm::vec3(0.60f, -0.05f, 0.0f));
            model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(0.90f, 0.03f, 0.22f));

            size_t idx = m_Objects.size();
            m_Objects.push_back({ model, model, WOOD_MEDIUM,
                "fan" + id + "_blade" + std::to_string(b) });
            m_FanBladeIndices.push_back(idx);
        }
    };

    int fanIdx = 0;
    for (float cz : fanZ)
        for (float cx : fanX)
            AddFan(cx, cz, fanIdx++);
}

// =============================================================================
// UpdateAnimations — Spin fan blades, animate interactives (door/windows)
// =============================================================================
void Scene::UpdateAnimations(float fanDeltaTime, float globalDeltaTime) {
    // ---- 1. Fan Animation (continuous) — all 9 fans spin in sync ----
    m_FanAngle += m_FanSpeed * fanDeltaTime;
    if (m_FanAngle > 360.0f) m_FanAngle -= 360.0f;

    // Each fan k occupies blade slots [k*4 .. k*4+3]
    const int numFans = static_cast<int>(m_FanCenters.size());
    for (int k = 0; k < numFans; k++) {
        const glm::vec3& center = m_FanCenters[k];
        for (int b = 0; b < 4; b++) {
            float bladeAngle = m_FanAngle + static_cast<float>(b) * 90.0f;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, center);
            model = glm::rotate(model, glm::radians(bladeAngle), glm::vec3(0, 1, 0));
            model = glm::translate(model, glm::vec3(0.60f, -0.05f, 0.0f));
            model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(0.90f, 0.03f, 0.22f));

            m_Objects[m_FanBladeIndices[k * 4 + b]].Transform = model;
        }
    }

    // ---- 2. Globe Rotation ----
    m_GlobeAngle += m_GlobeSpeed * globalDeltaTime;
    if (m_GlobeAngle > 360.0f) m_GlobeAngle -= 360.0f;
    for (auto& obj : m_CurvedObjects) {
        if (obj.Label == "globe") {
            using namespace glm;
            // Globe now on librarian desk: (gx=6.1, Y=1.46, gz=5.65)
            obj.Transform = translate(mat4(1.0f), vec3(6.1f, 1.46f, 5.65f))
                          * rotate(mat4(1.0f), radians(-23.5f), vec3(0.0f, 0.0f, 1.0f))
                          * rotate(mat4(1.0f), radians(m_GlobeAngle), vec3(0.0f, 1.0f, 0.0f))
                          * scale(mat4(1.0f), vec3(0.28f, 0.28f, 0.28f));
            break;
        }
    }

    // ---- 3. Door Animation (interactive) ----
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

        // DOOR PANEL — wood texture with color blend
        else if (obj.Label == "door") {
            obj.TextureID   = TextureManager::Get().GetID("door");
            obj.Mode        = TextureMode::FRAGMENT_BLEND;
            obj.UVTileX     = 1.0f;
            obj.UVTileY     = 1.0f;
            obj.Color       = LibraryColors::WOOD_MEDIUM;
            obj.BlendFactor = 0.25f;  // Mostly texture, slight wood tint
        }

        // Everything else remains FLAT_COLOR (Phase 5 behavior unchanged)
    }

    LOG_INFO("AssignTextures: Texture modes assigned to scene objects.");
}

// =============================================================================
// BuildPendantLamps — 6 pendant lights at 'L' positions in FLFLF grid
// =============================================================================
// Light L columns: X = -4.0 and X = +4.0
// Rows: Z = -6.0, 0.0, +6.0
// Total: 2 columns x 3 rows = 6 lamps
// Cones are added in BuildCurvedObjects.
// =============================================================================
void Scene::BuildPendantLamps() {
    const float lampX[] = { -4.0f, +4.0f };
    const float lampZ[] = { -6.0f,  0.0f, +6.0f };

    int lampIdx = 0;
    for (float cz : lampZ) {
        for (float cx : lampX) {
            std::string id = std::to_string(lampIdx++);
            Add("lamp_" + id + "_cord",
                { cx, 5.3f, cz },
                { 0.03f, 0.4f, 0.03f },
                glm::vec3(0.15f, 0.12f, 0.10f));
        }
    }

    LOG_INFO("BuildPendantLamps: 6 pendant lamps built at FLFLF L-positions.");
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

    // ---- 1. GLOBE (textured sphere) — now on the librarian desk ----
    // Desk top Y = 0.96 (desk center 0.90 + half 0.06).
    // Globe sits at (gx=6.1, gz=5.65) — left-front quadrant of the desk.
    // Y offset from original table (top 0.83): +0.13 → globe center = 1.46.
    // Globe is tilted 23.5° (Earth's axial tilt) and rotates around its tilted axis.
    {
        const float gx = 6.1f, gz = 5.65f;

        CurvedObject globe;
        globe.Label     = "globe";
        globe.Color     = vec3(0.3f, 0.5f, 0.8f);  // Ocean blue fallback
        globe.Mode      = TextureMode::SIMPLE_TEXTURE;
        globe.TextureID = TextureManager::Get().GetID("earth");
        globe.Transform = translate(mat4(1.0f), vec3(gx, 1.46f, gz))
                        * rotate(mat4(1.0f), radians(-23.5f), vec3(0.0f, 0.0f, 1.0f))
                        * scale(mat4(1.0f), vec3(0.28f, 0.28f, 0.28f));
        globe.Mesh      = Primitives::CreateSphere(40, 40);
        m_CurvedObjects.push_back(std::move(globe));
    }

    // ---- 2. GLOBE STAND — Realistic curved meridian stand ----
    // Sits on librarian desk at (gx, gz). Y shifted +0.13 from original table.
    {
        const float gx = 6.1f, gz = 5.65f;
        const glm::vec3 standDark(0.18f, 0.15f, 0.14f);

        CurvedObject base;
        base.Label     = "globe_stand_base";
        base.Color     = standDark;
        base.Mode      = TextureMode::FLAT_COLOR;
        base.Transform = translate(mat4(1.0f), vec3(gx, 0.98f, gz))
                       * scale(mat4(1.0f), vec3(0.20f, 0.02f, 0.20f));
        base.Mesh      = Primitives::CreateSphere(16, 24);
        m_CurvedObjects.push_back(std::move(base));

        // Vertical rod: from base top (0.99) to meridian arm bottom (1.46-0.30=1.16). Center=1.07.
        Add("globe_stand_pole", {gx, 1.07f, gz}, {0.025f, 0.18f, 0.025f}, standDark);

        // Curved Meridian Ring — centered at globe center, tilted -23.5°.
        CurvedObject arm;
        arm.Label     = "globe_stand_meridian";
        arm.Color     = standDark;
        arm.Mode      = TextureMode::FLAT_COLOR;
        arm.Transform = translate(mat4(1.0f), vec3(gx, 1.46f, gz))
                      * rotate(mat4(1.0f), radians(-23.5f), vec3(0.0f, 0.0f, 1.0f));
        arm.Mesh      = Primitives::CreateHalfTorus(0.30f, 0.015f, 16, 32);
        m_CurvedObjects.push_back(std::move(arm));

        // North Pole pin
        glm::mat4 np = translate(mat4(1.0f), vec3(gx, 1.46f, gz))
                     * rotate(mat4(1.0f), radians(-23.5f), vec3(0.0f, 0.0f, 1.0f))
                     * translate(mat4(1.0f), vec3(0.0f, 0.29f, 0.0f))
                     * scale(mat4(1.0f), vec3(0.012f, 0.02f, 0.012f));
        Add("globe_stand_pin_n", glm::vec3(np[3]), {0.012f, 0.02f, 0.012f}, standDark);
        m_Objects.back().Transform = np;
        m_Objects.back().OriginalTransform = np;

        // South Pole pin
        glm::mat4 sp = translate(mat4(1.0f), vec3(gx, 1.46f, gz))
                     * rotate(mat4(1.0f), radians(-23.5f), vec3(0.0f, 0.0f, 1.0f))
                     * translate(mat4(1.0f), vec3(0.0f, -0.29f, 0.0f))
                     * scale(mat4(1.0f), vec3(0.012f, 0.02f, 0.012f));
        Add("globe_stand_pin_s", glm::vec3(sp[3]), {0.012f, 0.02f, 0.012f}, standDark);
        m_Objects.back().Transform = sp;
        m_Objects.back().OriginalTransform = sp;
    }

    // ---- 3. PENDANT LAMP CONES — 6 cones at L-positions in FLFLF grid ----
    // L positions: X = {-4, +4}, Z = {-6, 0, +6}
    {
        const float lampX[] = { -4.0f, +4.0f };
        const float lampZ[] = { -6.0f,  0.0f, +6.0f };
        int lampIdx = 0;
        for (float cz : lampZ) {
            for (float cx : lampX) {
                CurvedObject cone;
                cone.Label   = "lamp_" + std::to_string(lampIdx++) + "_cone";
                cone.Color   = vec3(0.55f, 0.45f, 0.25f);  // Warm tan/brass
                cone.Mode    = TextureMode::FLAT_COLOR;
                cone.Transform = translate(mat4(1.0f), vec3(cx, 4.84f, cz))
                               * scale(mat4(1.0f), vec3(0.25f, 0.22f, 0.25f));
                cone.Mesh    = Primitives::CreateCone(32);
                m_CurvedObjects.push_back(std::move(cone));
            }
        }
    }

    // ---- 4. DECORATIVE BEZIER VASES — two flanking the door frame ----
    // All vases: scale (0.42, 1.2, 0.42) → belly ~0.21 world radius, lip at Y=1.2.
    // Fractal trees start at lip Y=1.2 for each vase.
    //
    // Door opening: Z ∈ [2.5, 4.5] on the right wall (X=+8).
    //   vase_door_lo  — Z=1.8 (low-Z flank, just below door frame)
    //   vase_door_hi  — Z=5.2 (high-Z flank, just above door frame)
    {
        struct VaseDef { const char* label; float x, z; };
        const VaseDef vaseDefs[] = {
            { "vase_door_lo", 11.5f, 1.8f },
            { "vase_door_hi", 11.5f, 5.2f },
        };
        for (const auto& v : vaseDefs) {
            CurvedObject vase;
            vase.Label   = v.label;
            vase.Color   = vec3(0.70f, 0.36f, 0.22f);  // Warm terracotta ceramic
            vase.Mode    = TextureMode::FLAT_COLOR;
            vase.Transform = translate(mat4(1.0f), vec3(v.x, 0.0f, v.z))
                           * scale(mat4(1.0f), vec3(0.42f, 1.2f, 0.42f));
            vase.Mesh    = Primitives::CreateBezierVase(60, 72);
            m_CurvedObjects.push_back(std::move(vase));
        }
    }

    // ---- 5. STUDY LAMP SHADE (cone) — desk lamp on librarian desk ----
    // Physical lamp model built in BuildLibrarianDesk() (cube arms + base).
    // The shade is tilted toward the reading surface so the head and spot beam
    // visually align with the upper arm and desk target area.
    // Position: (8.80, 1.71, 5.50) — tip of the upper lamp arm.
    {
        CurvedObject shade;
        shade.Label   = "desk_lamp_shade";
        shade.Color   = vec3(0.16f, 0.16f, 0.18f);  // Dark metal shade
        shade.Mode    = TextureMode::FLAT_COLOR;
        shade.Transform = translate(mat4(1.0f), vec3(8.80f, 1.71f, 5.50f))
                        * rotate(mat4(1.0f), radians(-20.0f), vec3(0.0f, 0.0f, 1.0f))
                        * scale(mat4(1.0f), vec3(0.20f, 0.14f, 0.20f));
        shade.Mesh    = Primitives::CreateCone(32);
        m_CurvedObjects.push_back(std::move(shade));
    }

    LOG_INFO("BuildCurvedObjects: globe, 6 lamp cones, 2 vases, 1 desk lamp shade created.");
}

// =============================================================================
// BuildFractalTree — Recursive fractal plant growing from the decorative vase
// =============================================================================
// Algorithm: iterative DFS (avoids deep call-stack on Windows).
// Each branch is an axis-aligned unit cube rotated so its local +Y aligns with
// the branch direction, then scaled to (thickness × length × thickness).
//
// At depth==0 the main tree calls AddFernFrond, which itself runs a depth-2
// mini-fractal of tiny twig branches. The tips of those mini-branches are FLAT
// leaf quads (scale X=width, Y=length, Z≈0.005 — one dimension near-zero),
// each "twisted" around its length axis so adjacent leaves face different
// directions — producing a self-similar fern frond that visibly shows the
// fractal branching at the leaf scale, not just cubes.
//
// Two vases, all scale (0.42, 1.2, 0.42) → lip at world Y = 1.2:
//   vase_door_lo  (7.6, 0, 1.8) — low-Z door flank
//   vase_door_hi  (7.6, 0, 5.2) — high-Z door flank
//
// Object count per plant (smaller trunk: len=0.30, thick=0.036):
//   Main branches (depth 5 tree): 31
//   Fronds per terminal (depth 2): 3 mini-branches + 8 flat leaf blades = 11
//   Per plant: 31 + 32 × 11 = 383  |  2 plants total: ~766 objects
// =============================================================================
void Scene::BuildFractalTree() {
    using namespace glm;

    // -----------------------------------------------------------------
    // BranchMat: model matrix for an elongated cube acting as one branch.
    // Rotates the cube's local +Y to face 'dir', centers it between
    // 'base' and 'base + dir*len', scales cross-section to 'thick'.
    // -----------------------------------------------------------------
    auto BranchMat = [](vec3 base, vec3 dir, float len, float thick) -> mat4 {
        vec3 center = base + dir * (len * 0.5f);
        vec3 yAxis(0.0f, 1.0f, 0.0f);

        mat4 T = translate(mat4(1.0f), center);

        vec3  axis = cross(yAxis, dir);
        float sinA = length(axis);
        float cosA = dot(yAxis, dir);
        mat4 R;
        if (sinA > 0.001f)
            R = rotate(mat4(1.0f), std::atan2(sinA, cosA), normalize(axis));
        else
            R = (cosA > 0.0f) ? mat4(1.0f)
                              : rotate(mat4(1.0f), 3.14159265f, vec3(1.0f, 0.0f, 0.0f));

        mat4 S = scale(mat4(1.0f), vec3(thick, len, thick));
        return T * R * S;
    };

    // -----------------------------------------------------------------
    // LeafMat: flat leaf-blade transform.
    // The blade is elongated along 'dir' (Y after rotation), wide in X,
    // and nearly flat in Z (thickness ≈ 0.005). A 'twist' angle rotates
    // the blade around its length axis so adjacent leaves face different
    // directions — giving natural, non-planar leaf arrangement.
    // -----------------------------------------------------------------
    auto LeafMat = [](vec3 base, vec3 dir, float len, float width, float twist) -> mat4 {
        vec3 center = base + dir * (len * 0.5f);
        vec3 yAxis(0.0f, 1.0f, 0.0f);
        mat4 T = translate(mat4(1.0f), center);

        vec3  axis = cross(yAxis, dir);
        float sinA = length(axis);
        float cosA = dot(yAxis, dir);
        mat4 R;
        if (sinA > 0.001f)
            R = rotate(mat4(1.0f), std::atan2(sinA, cosA), normalize(axis));
        else
            R = (cosA > 0.0f) ? mat4(1.0f)
                              : rotate(mat4(1.0f), 3.14159265f, vec3(1.0f, 0.0f, 0.0f));

        // Twist around the local length axis (Y in unrotated space),
        // then align that axis to dir — gives each leaf its own face direction
        mat4 Tw = rotate(mat4(1.0f), twist, yAxis);
        mat4 S  = scale(mat4(1.0f), vec3(width, len, 0.005f)); // almost-2D leaf blade
        return T * R * Tw * S;
    };

    // Branch colours (darker toward trunk, lighter toward twigs)
    const vec3 colTrunk (0.42f, 0.25f, 0.10f);
    const vec3 colBranch(0.32f, 0.18f, 0.08f);
    const vec3 colTwig  (0.22f, 0.13f, 0.06f);

    // Leaf colours — three shades of green for variety
    const vec3 colLeaf1 (0.14f, 0.54f, 0.18f);  // Mid green
    const vec3 colLeaf2 (0.22f, 0.66f, 0.10f);  // Bright lime green
    const vec3 colLeaf3 (0.09f, 0.42f, 0.22f);  // Deep forest green
    const vec3 leafPalette[3] = { colLeaf1, colLeaf2, colLeaf3 };

    int counter = 0;

    // -----------------------------------------------------------------
    // AddFernFrond: depth-2 mini-fractal placed at each main tree terminal.
    //
    // Replaces the old flat cube leaf clusters. Each frond is a self-similar
    // miniature of the main tree structure — showing fractal geometry at the
    // leaf scale. Frond tips are flat leaf blades (not cubes).
    //
    //  frond depth 2 → 1 mini-stem
    //  frond depth 1 → 2 mini-twigs
    //  frond depth 0 → 4 flat leaf blades (total per frond: 3 branches + 4 leaves)
    // -----------------------------------------------------------------
    auto AddFernFrond = [&](vec3 pos, vec3 stemDir) {
        struct MiniTask { vec3 base, dir; float len, thick; int depth; };
        std::vector<MiniTask> miniStack;
        // Initial frond stem: short and thin — matches the scale of the
        // terminal twigs of the main tree after 5 levels of taper
        miniStack.push_back({ pos, stemDir, 0.07f, 0.007f, 2 });

        // Split axes for the frond — perpendicular to the main tree axes
        // to give 3-D spread at the frond level as well
        const vec3 miniSplitA(0.0f, 0.0f, 1.0f);
        const vec3 miniSplitB(1.0f, 0.0f, 0.0f);
        int leafIdx = 0;

        while (!miniStack.empty()) {
            MiniTask mt = miniStack.back();
            miniStack.pop_back();

            if (mt.depth == 0) {
                // Two flat leaf blades per tip — second rotated 70° for denser foliage
                for (int li = 0; li < 2; ++li) {
                    float twist = radians(float(leafIdx) * 55.0f + float(li) * 70.0f);
                    SceneObject leaf;
                    leaf.Label             = "tree_leaf_" + std::to_string(counter++);
                    leaf.Color             = leafPalette[(leafIdx + li) % 3];
                    leaf.Mode              = TextureMode::FLAT_COLOR;
                    leaf.Transform         = LeafMat(mt.base, mt.dir,
                                                     mt.len * 1.6f,
                                                     0.065f,
                                                     twist);
                    leaf.OriginalTransform = leaf.Transform;
                    m_Objects.push_back(leaf);
                }
                ++leafIdx;
                continue;
            }

            // Mini twig branch (same cube mechanism as main tree)
            SceneObject twig;
            twig.Label             = "tree_leaf_stem_" + std::to_string(counter++);
            twig.Color             = colTwig;
            twig.Mode              = TextureMode::FLAT_COLOR;
            twig.Transform         = BranchMat(mt.base, mt.dir, mt.len, mt.thick);
            twig.OriginalTransform = twig.Transform;
            m_Objects.push_back(twig);

            vec3  endPt = mt.base + mt.dir * mt.len;
            // Wider split angle than main tree → fern-like open fan
            float angle = radians(38.0f);
            vec3 dirA = normalize(vec3(rotate(mat4(1.0f),  angle, miniSplitA) * vec4(mt.dir, 0.0f)));
            vec3 dirB = normalize(vec3(rotate(mat4(1.0f), -angle, miniSplitB) * vec4(mt.dir, 0.0f)));

            miniStack.push_back({ endPt, dirA, mt.len * 0.65f, mt.thick * 0.65f, mt.depth - 1 });
            miniStack.push_back({ endPt, dirB, mt.len * 0.65f, mt.thick * 0.65f, mt.depth - 1 });
        }
    };

    // -----------------------------------------------------------------
    // Plant positions — vase scale Y=1.2 → lip at world Y=1.2 for both.
    // Two flanking the door frame on each side.
    // Door opening: Z ∈ [2.5, 4.5] on right wall.
    // -----------------------------------------------------------------
    const glm::vec3 plantBases[] = {
        glm::vec3(11.5f, 1.2f, 1.8f),  // low-Z door flank (below Z=2.5 frame)
        glm::vec3(11.5f, 1.2f, 5.2f),  // high-Z door flank (above Z=4.5 frame)
    };

    // -----------------------------------------------------------------
    // Main tree: iterative DFS per plant — avoids stack overflow on Windows
    // -----------------------------------------------------------------
    struct Task { vec3 base, dir; float len, thick; int depth; };
    const vec3 splitAxisA(0.0f, 0.0f, 1.0f);  // Child A tilts in XY plane (around Z)
    const vec3 splitAxisB(1.0f, 0.0f, 0.0f);  // Child B tilts in YZ plane (around X)

    for (const vec3& treeBase : plantBases) {
        std::vector<Task> taskStack;
        taskStack.reserve(128);
        // Smaller trunk: len=0.30 (was 0.45), thick=0.036 (was 0.055)
        taskStack.push_back({ treeBase, vec3(0.0f, 1.0f, 0.0f), 0.30f, 0.036f, 5 });

        while (!taskStack.empty()) {
            Task t = taskStack.back();
            taskStack.pop_back();

            if (t.depth == 0) {
                // Fern frond (depth-2 mini-fractal with flat leaf blades) at branch tip
                AddFernFrond(t.base, t.dir);
                continue;
            }

            SceneObject branch;
            branch.Label             = "tree_branch_" + std::to_string(counter++);
            branch.Color             = (t.depth >= 4) ? colTrunk
                                     : (t.depth >= 2) ? colBranch
                                     :                  colTwig;
            branch.Mode              = TextureMode::FLAT_COLOR;
            branch.Transform         = BranchMat(t.base, t.dir, t.len, t.thick);
            branch.OriginalTransform = branch.Transform;
            m_Objects.push_back(branch);

            vec3  endPt  = t.base + t.dir * t.len;
            float cLen   = t.len   * 0.67f;   // each child 67% of parent length
            float cThick = t.thick * 0.65f;   // taper cross-section
            // Spread angle opens slightly wider at shallower depths
            float angle  = radians(26.0f + float(5 - t.depth) * 2.0f);

            vec3 dirA = normalize(vec3(rotate(mat4(1.0f),  angle, splitAxisA) * vec4(t.dir, 0.0f)));
            vec3 dirB = normalize(vec3(rotate(mat4(1.0f),  angle, splitAxisB) * vec4(t.dir, 0.0f)));

            taskStack.push_back({ endPt, dirA, cLen, cThick, t.depth - 1 });
            taskStack.push_back({ endPt, dirB, cLen, cThick, t.depth - 1 });
        }
    }

    LOG_INFO("BuildFractalTree: " + std::to_string(counter) + " tree objects across 2 plants.");
}

// =============================================================================
// BuildLibrarianDesk — Posh reception/librarian desk at (cx=7.5, cz=6.0)
// =============================================================================
// Replaces the removed front-right table+chair set and bookshelf.
// Components:
//   1. Wide mahogany desk top (4.5 × 0.12 × 1.8), top surface at Y=0.96
//   2. Patron-facing modesty panel with 3 drawer units + handles
//   3. Left/right end panels
//   4. Raised privacy partition on librarian's side
//   5. 4 thick legs
//   6. Executive chair behind the desk (dark green leather, high backrest)
//   7. Papers, open book, and stacked reference books on desk surface
//   8. Study lamp (arm + base — cone shade is a CurvedObject in BuildCurvedObjects)
//      Spotlight source sits inside the tilted cone near Y=1.67.
// =============================================================================
void Scene::BuildLibrarianDesk() {
    const size_t deskBodyStart = m_Objects.size();
    const glm::mat4 deskBodyRotation = RotateAroundPivotY(LIBRARIAN_DESK_CENTER, 180.0f);

    // Colors
    const glm::vec3 maho    = WOOD_MAHOGANY;
    const glm::vec3 trim    = DESK_TRIM;
    const glm::vec3 brass   = BRASS;
    const glm::vec3 green   = CUSHION_GREEN;
    const glm::vec3 paper   = PAPER_WHITE;
    const glm::vec3 metal   = LAMP_METAL;

    const float cx = 7.5f, cz = 6.0f;   // desk center
    const float deskTopY = 0.96f;        // top surface of the desk

    // ---- 1. Desk top ----
    Add("desk_top",  {cx, 0.90f, cz}, {4.5f, 0.12f, 1.8f}, maho);

    // ---- 2. Patron-facing front panel (modesty panel) ----
    // Placed at Z = cz - 0.90 = 5.10, spans full desk width.
    Add("desk_front_panel", {cx, 0.47f, 5.10f}, {4.5f, 0.94f, 0.08f}, maho);

    // Drawer faces (3 units on the front panel, slightly proud of it)
    const float drawY = 0.42f;
    const float drawZ = 5.065f;
    Add("desk_drawer_L",  {6.0f, drawY, drawZ}, {1.10f, 0.26f, 0.04f}, trim);
    Add("desk_drawer_C",  {7.5f, drawY, drawZ}, {1.10f, 0.26f, 0.04f}, trim);
    Add("desk_drawer_R",  {9.0f, drawY, drawZ}, {1.10f, 0.26f, 0.04f}, trim);

    // Drawer handles (brass)
    Add("desk_handle_L",  {6.0f, drawY, 5.043f}, {0.22f, 0.03f, 0.025f}, brass);
    Add("desk_handle_C",  {7.5f, drawY, 5.043f}, {0.22f, 0.03f, 0.025f}, brass);
    Add("desk_handle_R",  {9.0f, drawY, 5.043f}, {0.22f, 0.03f, 0.025f}, brass);

    // Nameplate (brass strip near top of front panel)
    Add("desk_nameplate", {cx, 0.80f, 5.055f}, {0.85f, 0.09f, 0.025f}, brass);

    // ---- 3. Left and right end panels ----
    Add("desk_end_L", {5.27f, 0.47f, cz}, {0.06f, 0.94f, 1.8f}, maho);
    Add("desk_end_R", {9.73f, 0.47f, cz}, {0.06f, 0.94f, 1.8f}, maho);

    // ---- 4. Raised privacy partition on librarian's side ----
    // Sits on top of the desk at Z = 6.90 (back edge of desk surface).
    Add("desk_partition", {cx, 1.185f, 6.90f}, {4.30f, 0.45f, 0.07f}, maho);
    // Partition cap trim
    Add("desk_partition_cap", {cx, 1.415f, 6.90f}, {4.30f, 0.04f, 0.09f}, trim);

    // ---- 5. Four thick legs ----
    const float legH = 0.84f, legW = 0.10f;
    const float legY = legH * 0.5f;
    Add("desk_leg_FL", {5.35f, legY, 5.25f}, {legW, legH, legW}, WOOD_DARK);
    Add("desk_leg_FR", {9.65f, legY, 5.25f}, {legW, legH, legW}, WOOD_DARK);
    Add("desk_leg_BL", {5.35f, legY, 6.75f}, {legW, legH, legW}, WOOD_DARK);
    Add("desk_leg_BR", {9.65f, legY, 6.75f}, {legW, legH, legW}, WOOD_DARK);

    const size_t deskBodyEnd = m_Objects.size();
    for (size_t i = deskBodyStart; i < deskBodyEnd; ++i) {
        m_Objects[i].Transform = deskBodyRotation * m_Objects[i].Transform;
        m_Objects[i].OriginalTransform = deskBodyRotation * m_Objects[i].OriginalTransform;
    }

    // ---- 6. Executive chair ----
    // Chair sits at Z=7.55 (0.65 behind desk back edge 6.90), facing -Z (toward desk).
    {
        const float chX = cx, chZ = 7.60f;

        // Seat
        Add("exec_seat_frame",   {chX, 0.48f, chZ}, {0.68f, 0.06f, 0.68f}, trim);
        Add("exec_seat_cushion", {chX, 0.52f, chZ}, {0.60f, 0.08f, 0.60f}, green);

        // High backrest — extends well above head height for exec look
        // Backrest back face at Z = chZ + 0.25 = 7.85, center Z = 7.82
        Add("exec_back_frame",   {chX, 1.22f, 7.82f}, {0.68f, 1.28f, 0.07f}, trim);
        Add("exec_back_cushion", {chX, 1.22f, 7.80f}, {0.60f, 1.15f, 0.06f}, green);
        // Headrest bump at top
        Add("exec_headrest",     {chX, 1.90f, 7.83f}, {0.52f, 0.22f, 0.09f}, green);

        // Armrests
        Add("exec_arm_L", {chX - 0.38f, 0.68f, chZ}, {0.05f, 0.05f, 0.60f}, trim);
        Add("exec_arm_R", {chX + 0.38f, 0.68f, chZ}, {0.05f, 0.05f, 0.60f}, trim);
        // Arm support posts
        Add("exec_armpost_L", {chX - 0.38f, 0.56f, chZ - 0.20f}, {0.05f, 0.24f, 0.05f}, WOOD_DARK);
        Add("exec_armpost_R", {chX + 0.38f, 0.56f, chZ - 0.20f}, {0.05f, 0.24f, 0.05f}, WOOD_DARK);

        // 4 legs
        Add("exec_leg_FL", {chX - 0.28f, 0.24f, chZ - 0.28f}, {0.06f, 0.48f, 0.06f}, WOOD_DARK);
        Add("exec_leg_FR", {chX + 0.28f, 0.24f, chZ - 0.28f}, {0.06f, 0.48f, 0.06f}, WOOD_DARK);
        Add("exec_leg_BL", {chX - 0.28f, 0.24f, chZ + 0.28f}, {0.06f, 0.48f, 0.06f}, WOOD_DARK);
        Add("exec_leg_BR", {chX + 0.28f, 0.24f, chZ + 0.28f}, {0.06f, 0.48f, 0.06f}, WOOD_DARK);
    }

    // ---- 7. Desk surface items ----
    // Papers (thin flat slabs lying on the desk)
    Add("desk_paper_A",   {6.70f, deskTopY + 0.005f, 5.85f},
        {0.0f, 12.0f, 0.0f},   // slight rotation around Y
        {0.90f, 0.008f, 0.70f}, paper);
    Add("desk_paper_B",   {6.50f, deskTopY + 0.010f, 5.95f},
        {0.0f, -8.0f, 0.0f},
        {0.75f, 0.008f, 0.58f}, glm::vec3(0.88f, 0.90f, 0.82f));  // slightly tinted

    // Open book (two pages + spine)
    const float bookY = deskTopY + 0.008f;
    Add("desk_book_pg_L",  {6.05f, bookY, 6.20f}, {0.40f, 0.015f, 0.52f}, paper);
    Add("desk_book_pg_R",  {6.50f, bookY, 6.20f}, {0.40f, 0.015f, 0.52f}, paper);
    Add("desk_book_spine", {6.275f, bookY - 0.004f, 6.20f}, {0.05f, 0.015f, 0.52f}, WOOD_DARK);

    // Stacked reference books on librarian side of desk
    Add("desk_refbook_A",  {8.10f, deskTopY + 0.02f, 6.30f}, {0.32f, 0.04f, 0.46f}, BOOK_RED);
    Add("desk_refbook_B",  {8.10f, deskTopY + 0.06f, 6.30f}, {0.30f, 0.04f, 0.44f}, BOOK_BLUE);
    // Standing book with spine visible
    Add("desk_refbook_C",  {8.42f, deskTopY + 0.12f, 6.10f},
        {0.0f, 8.0f, 0.0f},
        {0.04f, 0.26f, 0.32f}, BOOK_GREEN);

    // ---- 8. Study Lamp (arm + base — cone shade is a CurvedObject) ----
    // Base disc at desk top right corner: (8.90, desk top, 5.50)
    const float lx = 8.90f, lz = 5.50f;
    Add("desk_lamp_base",       {lx,        deskTopY + 0.025f, lz},  {0.24f, 0.05f,  0.24f}, metal);
    Add("desk_lamp_joint_lo",   {lx,        deskTopY + 0.065f, lz},  {0.07f, 0.07f,  0.07f}, metal);
    Add("desk_lamp_arm_lo",     {lx,        1.225f,            lz},  {0.04f, 0.37f,  0.04f}, metal);
    Add("desk_lamp_elbow",      {lx,        1.435f,            lz},  {0.08f, 0.08f,  0.08f}, metal);
    // Upper arm tilted +20° toward desk center (Rz=+20)
    Add("desk_lamp_arm_hi",     {8.85f,     1.575f,            lz},
        {0.0f, 0.0f, 20.0f},
        {0.04f, 0.30f, 0.04f}, metal);
    // Bulb hint (tiny warm cube inside shade)
    Add("desk_lamp_bulb",       {8.80f,     1.66f,             lz},  {0.05f, 0.05f,  0.05f},
        glm::vec3(1.0f, 0.95f, 0.70f));

    LOG_INFO("BuildLibrarianDesk: desk, exec chair, and study lamp built at (7.5, 6.0).");
}
