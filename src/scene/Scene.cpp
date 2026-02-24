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
}

// =============================================================================
// SetCameraMatrices
// =============================================================================
void Scene::SetCameraMatrices(const glm::mat4& view, const glm::mat4& projection) {
    m_View       = view;
    m_Projection = projection;
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
// =============================================================================
void Scene::BuildRoom() {
    float fullW = ROOM_HALF_W * 2.0f;  // 16
    float fullD = ROOM_HALF_D * 2.0f;  // 14
    float midY  = ROOM_HEIGHT * 0.5f;  // 3

    // Floor
    Add("floor",      {0, 0, 0},                    {fullW, WALL_THICK, fullD},     FLOOR_TILE);
    // Ceiling
    Add("ceiling",    {0, ROOM_HEIGHT, 0},           {fullW, WALL_THICK, fullD},     CEILING_WOOD);

    // Left wall (X = -8)
    Add("wall_left",  {-ROOM_HALF_W, midY, 0},      {WALL_THICK, ROOM_HEIGHT, fullD},  WALL_CREAM);
    // Right wall (X = +8)
    Add("wall_right", { ROOM_HALF_W, midY, 0},      {WALL_THICK, ROOM_HEIGHT, fullD},  WALL_CREAM);
    // Back wall (Z = -7)
    Add("wall_back",  {0, midY, -ROOM_HALF_D},      {fullW, ROOM_HEIGHT, WALL_THICK},  WALL_CREAM);
    // Front wall (Z = +7)
    Add("wall_front", {0, midY,  ROOM_HALF_D},      {fullW, ROOM_HEIGHT, WALL_THICK},  WALL_CREAM);
}

// =============================================================================
// BuildWindows — Simple rectangular window: glass pane + wooden border on all 4 sides
// =============================================================================
void Scene::BuildWindows() {
    float leftX  = -ROOM_HALF_W + 0.11f;   // Inner face of left wall
    float rightX =  ROOM_HALF_W - 0.11f;   // Inner face of right wall

    // Window geometry constants
    float wZ   = -1.5f;       // Z center of both windows
    float wCy  = 2.8f;        // Vertical center of glass
    float gW   = 2.2f;        // Glass width (along Z)
    float gH   = 2.6f;        // Glass height (along Y)
    float bT   = 0.15f;        // Border thickness
    float gT   = 0.06f;        // Glass depth (thin slab)
    float bD   = 0.10f;        // Border depth (slightly proud of glass)

    // Helper: build one window (glass + 4 borders) at given X face
    auto BuildWindow = [&](const std::string& name, float wx, float faceSign) {
        // Glass pane
        Add(name + "_glass",
            {wx, wCy, wZ},
            {gT, gH, gW}, WINDOW_TEAL);

        // Top border
        Add(name + "_top",
            {wx - faceSign * 0.01f, wCy + gH * 0.5f + bT * 0.5f, wZ},
            {bD, bT, gW + bT * 2.0f}, WOOD_DARK);

        // Bottom border
        Add(name + "_bot",
            {wx - faceSign * 0.01f, wCy - gH * 0.5f - bT * 0.5f, wZ},
            {bD, bT, gW + bT * 2.0f}, WOOD_DARK);

        // Left border  (low Z)
        Add(name + "_left",
            {wx - faceSign * 0.01f, wCy, wZ - gW * 0.5f - bT * 0.5f},
            {bD, gH, bT}, WOOD_DARK);

        // Right border (high Z)
        Add(name + "_right",
            {wx - faceSign * 0.01f, wCy, wZ + gW * 0.5f + bT * 0.5f},
            {bD, gH, bT}, WOOD_DARK);
    };

    BuildWindow("window_left",  leftX,  -1.0f);   // Left wall: face points inward (+X)
    BuildWindow("window_right", rightX, +1.0f);   // Right wall: face points inward (-X)
}


// =============================================================================
// BuildDoor — Wooden door on right wall
// =============================================================================
void Scene::BuildDoor() {
    float doorX = ROOM_HALF_W - 0.1f;  // Inner face of right wall

    Add("door_frame",  {doorX, 2.0f, 3.5f},    {0.12f, 4.2f, 2.4f},   WOOD_DARK);
    Add("door",        {doorX, 1.9f, 3.5f},    {0.16f, 3.8f, 2.0f},   WOOD_MEDIUM);
    Add("door_handle", {doorX - 0.12f, 1.8f, 2.6f}, {0.15f, 0.08f, 0.08f}, METAL_DARK);
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
        m_Objects.push_back({model, WOOD_MEDIUM, "fan_blade" + std::to_string(i)});
        m_FanBladeIndices.push_back(idx);
    }
}

// =============================================================================
// UpdateAnimations — Spin fan blades each frame
// =============================================================================
void Scene::UpdateAnimations(float deltaTime) {
    m_FanAngle += m_FanSpeed * deltaTime;
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
        else if (obj.Label.rfind("wall_", 0) == 0) {
            obj.TextureID   = wallTex;
            obj.Mode        = TextureMode::VERTEX_BLEND;
            obj.UVTileX     = 3.0f;
            obj.UVTileY     = 2.0f;
            obj.Color       = LibraryColors::WALL_CREAM;
            obj.BlendFactor = 0.35f;  // Subtle tint — mostly texture
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
