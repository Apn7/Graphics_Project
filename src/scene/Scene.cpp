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
#include "utils/Transform.h"
#include "utils/Logger.h"
#include "core/Shader.h"
#include "renderer/Mesh.h"

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
// BuildWindows — 2 arched windows flush on inner wall face
// =============================================================================
void Scene::BuildWindows() {
    // Windows sit on the INNER face of the wall (inset slightly from wall center)
    float leftX  = -ROOM_HALF_W + 0.12f;   // Inner face of left wall
    float rightX =  ROOM_HALF_W - 0.12f;   // Inner face of right wall

    // --- Left wall window ---
    Add("window_left_frame", {leftX - 0.02f, 2.8f, -1.5f},  {0.06f, 3.6f, 2.6f},   WINDOW_FRAME);
    Add("window_left_rect",  {leftX, 2.5f, -1.5f},          {0.08f, 3.0f, 2.2f},   WINDOW_TEAL);
    Add("window_left_arch",  {leftX, 4.2f, -1.5f},          {0.08f, 1.0f, 1.4f},   WINDOW_TEAL);

    // --- Right wall window (mirrored) ---
    Add("window_right_frame", {rightX + 0.02f, 2.8f, -1.5f}, {0.06f, 3.6f, 2.6f},  WINDOW_FRAME);
    Add("window_right_rect",  {rightX, 2.5f, -1.5f},         {0.08f, 3.0f, 2.2f},  WINDOW_TEAL);
    Add("window_right_arch",  {rightX, 4.2f, -1.5f},         {0.08f, 1.0f, 1.4f},  WINDOW_TEAL);
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
// BuildTables — 3 reading tables
// =============================================================================
void Scene::BuildTables() {
    auto BuildTable = [&](const std::string& prefix, glm::vec3 center) {
        Add(prefix + "_top",  {center.x, 0.78f, center.z},     {2.4f, 0.10f, 1.2f},   WOOD_TABLE);

        float lx = 1.0f, lz = 0.45f;
        Add(prefix + "_leg0", {center.x - lx, 0.38f, center.z - lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add(prefix + "_leg1", {center.x + lx, 0.38f, center.z - lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add(prefix + "_leg2", {center.x - lx, 0.38f, center.z + lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add(prefix + "_leg3", {center.x + lx, 0.38f, center.z + lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);

        Add(prefix + "_brace_a", {center.x, 0.12f, center.z}, {2.2f, 0.05f, 0.07f}, WOOD_MEDIUM);
        Add(prefix + "_brace_b", {center.x, 0.12f, center.z}, {0.07f, 0.05f, 1.0f}, WOOD_MEDIUM);
    };

    BuildTable("table_0", { 1.0f, 0.0f,  3.5f});   // Front area
    BuildTable("table_1", { 2.0f, 0.0f,  0.5f});   // Mid area
    BuildTable("table_2", { 3.0f, 0.0f, -2.5f});   // Back area
}

// =============================================================================
// BuildChairs — Blue-cushioned chairs around each table
// =============================================================================
void Scene::BuildChairs() {
    auto BuildChair = [&](const std::string& prefix, glm::vec3 pos, float yawDeg) {
        (void)yawDeg;

        Add(prefix + "_seat_frame", pos + glm::vec3(0, 0.48f, 0),
            {0.50f, 0.05f, 0.50f}, WOOD_MEDIUM);
        Add(prefix + "_cushion", pos + glm::vec3(0, 0.52f, 0),
            {0.44f, 0.06f, 0.44f}, CUSHION_BLUE);

        glm::vec3 backPos = pos + glm::vec3(0, 0.85f, -0.22f);
        Add(prefix + "_back_frame", backPos, {0.50f, 0.60f, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_back_cushion", backPos + glm::vec3(0, 0, 0.03f),
            {0.44f, 0.54f, 0.04f}, CUSHION_BLUE);

        float lx = 0.20f, lz = 0.20f, legH = 0.48f;
        Add(prefix + "_leg0", pos + glm::vec3(-lx, legH * 0.5f, -lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_leg1", pos + glm::vec3( lx, legH * 0.5f, -lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_leg2", pos + glm::vec3(-lx, legH * 0.5f,  lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
        Add(prefix + "_leg3", pos + glm::vec3( lx, legH * 0.5f,  lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);

        Add(prefix + "_arm_l", pos + glm::vec3(-0.27f, 0.65f, 0), {0.04f, 0.04f, 0.48f}, WOOD_MEDIUM);
        Add(prefix + "_arm_r", pos + glm::vec3( 0.27f, 0.65f, 0), {0.04f, 0.04f, 0.48f}, WOOD_MEDIUM);
    };

    // Chairs around table_0 (center: 1.0, 0, 3.5)
    BuildChair("chair_t0_front", { 1.0f, 0.0f, 5.0f},  180.0f);
    BuildChair("chair_t0_back",  { 1.0f, 0.0f, 2.2f},    0.0f);
    BuildChair("chair_t0_left",  {-0.5f, 0.0f, 3.5f},   90.0f);
    BuildChair("chair_t0_right", { 2.5f, 0.0f, 3.5f},  270.0f);

    // Chairs around table_1 (center: 2.0, 0, 0.5)
    BuildChair("chair_t1_front", { 2.0f, 0.0f, 2.0f},  180.0f);
    BuildChair("chair_t1_back",  { 2.0f, 0.0f, -0.8f},   0.0f);
    BuildChair("chair_t1_left",  { 0.5f, 0.0f, 0.5f},   90.0f);
    BuildChair("chair_t1_right", { 3.5f, 0.0f, 0.5f},  270.0f);

    // Chairs around table_2 (center: 3.0, 0, -2.5)
    BuildChair("chair_t2_front", { 3.0f, 0.0f, -1.0f},  180.0f);
    BuildChair("chair_t2_back",  { 3.0f, 0.0f, -3.8f},    0.0f);
    BuildChair("chair_t2_left",  { 1.5f, 0.0f, -2.5f},   90.0f);
    BuildChair("chair_t2_right", { 4.5f, 0.0f, -2.5f},  270.0f);
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
