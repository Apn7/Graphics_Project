// =============================================================================
// Scene.cpp — Library Scene Construction (Phase 5)
// =============================================================================
// Implements the complete library scene using transformed cubes.
// All objects use a single shared cube mesh — only the model matrix differs.
// No OpenGL calls here — only in Mesh::Draw().
// =============================================================================

#include "scene/Scene.h"
#include "scene/LibraryColors.h"
#include "renderer/Primitives.h"
#include "utils/Transform.h"
#include "utils/Logger.h"
#include "core/Shader.h"
#include "renderer/Mesh.h"

#include <cmath>      // cos, sin for fan blades

using namespace LibraryColors;

// =============================================================================
// Constructor
// =============================================================================
Scene::Scene()
    : m_CubeMesh(nullptr)
{
}

Scene::~Scene() = default;

// =============================================================================
// Add — Shorthand (no rotation)
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

// =============================================================================
// Add — Full (with rotation)
// =============================================================================
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
// Build — Constructs the entire library scene
// =============================================================================
void Scene::Build() {
    m_CubeMesh = Primitives::CreateCube();
    m_Objects.clear();

    BuildRoom();
    BuildWindows();
    BuildDoor();
    BuildWallShelves();
    BuildIslandShelf();
    BuildBooks();
    BuildTables();
    BuildChairs();
    BuildCeiling();

    LOG_INFO("Scene built: " + std::to_string(m_Objects.size()) + " objects");
}

// =============================================================================
// Render — Draw all objects
// =============================================================================
// TODO Phase 6: Upload u_Material per object instead of u_Color
void Scene::Render(Shader& shader) {
    for (const auto& obj : m_Objects) {
        shader.SetMat4("u_Model", obj.Transform);
        shader.SetVec3("u_Color", obj.Color);
        m_CubeMesh->Draw();
    }
}

// =============================================================================
// RenderGroup — Draw only objects matching a label prefix
// =============================================================================
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
void Scene::BuildRoom() {
    // Floor
    Add("floor",        {0.0f, 0.0f, 0.0f},    {12.0f, 0.2f, 10.0f},  FLOOR_TILE);

    // Ceiling
    Add("ceiling",      {0.0f, 4.5f, 0.0f},    {12.0f, 0.2f, 10.0f},  CEILING_WOOD);

    // Left wall (X = -6)
    Add("wall_left",    {-6.0f, 2.25f, 0.0f},  {0.2f, 4.5f, 10.0f},   WALL_CREAM);

    // Right wall (X = +6)
    Add("wall_right",   {6.0f, 2.25f, 0.0f},   {0.2f, 4.5f, 10.0f},   WALL_CREAM);

    // Back wall (Z = -5)
    Add("wall_back",    {0.0f, 2.25f, -5.0f},  {12.0f, 4.5f, 0.2f},   WALL_CREAM);

    // Front wall (Z = +5)
    Add("wall_front",   {0.0f, 2.25f, 5.0f},   {12.0f, 4.5f, 0.2f},   WALL_CREAM);
}

// =============================================================================
// BuildWindows — 2 arched windows (left + right walls)
// =============================================================================
void Scene::BuildWindows() {
    // --- Left wall window ---
    Add("window_left_frame", {-5.92f, 2.2f, -1.0f},  {0.05f, 3.0f, 2.2f},   WINDOW_FRAME);
    Add("window_left_rect",  {-5.9f, 2.0f, -1.0f},   {0.1f, 2.5f, 1.8f},    WINDOW_TEAL);
    Add("window_left_arch",  {-5.9f, 3.4f, -1.0f},   {0.1f, 0.8f, 1.2f},    WINDOW_TEAL);

    // --- Right wall window (mirrored) ---
    Add("window_right_frame", {5.92f, 2.2f, -1.0f},  {0.05f, 3.0f, 2.2f},   WINDOW_FRAME);
    Add("window_right_rect",  {5.9f, 2.0f, -1.0f},   {0.1f, 2.5f, 1.8f},    WINDOW_TEAL);
    Add("window_right_arch",  {5.9f, 3.4f, -1.0f},   {0.1f, 0.8f, 1.2f},    WINDOW_TEAL);
}

// =============================================================================
// BuildDoor — Wooden door on right wall
// =============================================================================
void Scene::BuildDoor() {
    Add("door_frame",   {5.9f, 1.9f, 2.5f},    {0.1f, 4.0f, 2.2f},    WOOD_DARK);
    Add("door",         {5.9f, 1.8f, 2.5f},     {0.15f, 3.6f, 1.8f},   WOOD_MEDIUM);
    Add("door_handle",  {5.8f, 1.8f, 1.7f},     {0.15f, 0.08f, 0.08f}, METAL_DARK);
}

// =============================================================================
// BuildWallShelves — Left wall (2 tall units) + back wall (3 medium units)
// =============================================================================
void Scene::BuildWallShelves() {
    // ---------------------------------------------------------------
    // Helper: one tall shelf unit on the left wall
    // ---------------------------------------------------------------
    auto BuildTallShelf = [&](float zCenter, const std::string& prefix) {
        float x = -5.5f;  // Hugging left wall

        Add(prefix + "_back",   {x, 2.0f, zCenter},                    {0.1f, 4.0f, 1.8f},    WOOD_DARK);
        Add(prefix + "_left",   {x + 0.85f, 2.0f, zCenter - 0.85f},   {0.1f, 4.0f, 0.1f},    WOOD_DARK);
        Add(prefix + "_right",  {x + 0.85f, 2.0f, zCenter + 0.85f},   {0.1f, 4.0f, 0.1f},    WOOD_DARK);
        Add(prefix + "_top",    {x + 0.45f, 4.0f, zCenter},           {0.9f, 0.08f, 1.8f},   WOOD_DARK);
        Add(prefix + "_base",   {x + 0.45f, 0.1f, zCenter},           {0.9f, 0.15f, 1.8f},   WOOD_DARK);

        // 5 shelves at even vertical intervals
        for (int i = 0; i < 5; i++) {
            float shelfY = 0.6f + i * 0.65f;
            Add(prefix + "_shelf" + std::to_string(i),
                {x + 0.45f, shelfY, zCenter},
                {0.9f, 0.06f, 1.8f}, WOOD_DARK);
        }
    };

    BuildTallShelf(-2.0f, "shelf_wall_left_A");
    BuildTallShelf( 0.5f, "shelf_wall_left_B");

    // ---------------------------------------------------------------
    // Helper: one medium shelf unit on the back wall
    // ---------------------------------------------------------------
    auto BuildMediumShelf = [&](float xCenter, const std::string& prefix) {
        float z = -4.7f;  // Hugging back wall

        Add(prefix + "_back",   {xCenter, 1.8f, z},                    {1.6f, 3.2f, 0.1f},    WOOD_DARK);
        Add(prefix + "_left",   {xCenter - 0.75f, 1.8f, z + 0.3f},   {0.08f, 3.2f, 0.6f},   WOOD_DARK);
        Add(prefix + "_right",  {xCenter + 0.75f, 1.8f, z + 0.3f},   {0.08f, 3.2f, 0.6f},   WOOD_DARK);
        Add(prefix + "_top",    {xCenter, 3.3f, z + 0.3f},            {1.6f, 0.07f, 0.6f},   WOOD_DARK);
        Add(prefix + "_base",   {xCenter, 0.1f, z + 0.3f},            {1.6f, 0.12f, 0.6f},   WOOD_DARK);

        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.65f;
            Add(prefix + "_shelf" + std::to_string(i),
                {xCenter, shelfY, z + 0.3f},
                {1.6f, 0.06f, 0.6f}, WOOD_DARK);
        }
    };

    BuildMediumShelf(-3.5f, "shelf_back_A");
    BuildMediumShelf( 0.0f, "shelf_back_B");
    BuildMediumShelf( 3.5f, "shelf_back_C");
}

// =============================================================================
// BuildIslandShelf — Freestanding shelf in center-left of room
// =============================================================================
void Scene::BuildIslandShelf() {
    float cx = -2.5f, cz = -1.5f;

    Add("island_left",  {cx - 0.75f, 1.8f, cz},   {0.08f, 3.2f, 1.8f},   WOOD_DARK);
    Add("island_right", {cx + 0.75f, 1.8f, cz},   {0.08f, 3.2f, 1.8f},   WOOD_DARK);
    Add("island_top",   {cx, 3.3f, cz},            {1.6f, 0.07f, 1.8f},   WOOD_DARK);
    Add("island_base",  {cx, 0.1f, cz},            {1.6f, 0.12f, 1.8f},   WOOD_DARK);

    for (int i = 0; i < 4; i++) {
        Add("island_shelf" + std::to_string(i),
            {cx, 0.55f + i * 0.65f, cz},
            {1.6f, 0.06f, 1.8f}, WOOD_DARK);
    }
}

// =============================================================================
// BuildBooks — Fill all shelf rows with colorful books
// =============================================================================
void Scene::BuildBooks() {
    const glm::vec3 bookColors[] = {
        BOOK_RED, BOOK_BLUE, BOOK_GREEN, BOOK_YELLOW,
        BOOK_ORANGE, BOOK_PURPLE, BOOK_WHITE, BOOK_BROWN
    };
    const int colorCount = 8;

    // ---------------------------------------------------------------
    // Helper: fills one shelf row with books
    // ---------------------------------------------------------------
    auto FillShelfRow = [&](const std::string& prefix,
                            glm::vec3 shelfPos, float shelfWidth, float shelfDepth) {
        float bookW = 0.07f;    // Spine width
        float bookH = 0.50f;    // Book height
        float bookD = shelfDepth * 0.85f;

        int count = static_cast<int>(shelfWidth / (bookW + 0.01f));
        float startX = shelfPos.x - shelfWidth * 0.5f + bookW * 0.5f;

        for (int i = 0; i < count; i++) {
            glm::vec3 bPos = {
                startX + i * (bookW + 0.01f),
                shelfPos.y + bookH * 0.5f + 0.03f,
                shelfPos.z
            };
            glm::vec3 bColor = bookColors[i % colorCount];
            Add(prefix + "_book" + std::to_string(i),
                bPos, {bookW, bookH, bookD}, bColor);
        }
    };

    // --- Left wall tall shelves (2 units × 5 shelves each) ---
    auto FillTallShelf = [&](float zCenter, const std::string& prefix) {
        float x = -5.05f;  // Center of shelf depth
        for (int i = 0; i < 5; i++) {
            float shelfY = 0.6f + i * 0.65f;
            FillShelfRow(prefix + "_s" + std::to_string(i),
                         {x, shelfY, zCenter}, 0.85f, 1.7f);
        }
    };

    FillTallShelf(-2.0f, "books_left_A");
    FillTallShelf( 0.5f, "books_left_B");

    // --- Back wall medium shelves (3 units × 4 shelves each) ---
    auto FillMediumShelf = [&](float xCenter, const std::string& prefix) {
        float z = -4.4f;
        for (int i = 0; i < 4; i++) {
            float shelfY = 0.55f + i * 0.65f;
            FillShelfRow(prefix + "_s" + std::to_string(i),
                         {xCenter, shelfY, z}, 1.5f, 0.5f);
        }
    };

    FillMediumShelf(-3.5f, "books_back_A");
    FillMediumShelf( 0.0f, "books_back_B");
    FillMediumShelf( 3.5f, "books_back_C");

    // --- Island shelf (4 shelves) ---
    float cx = -2.5f, cz = -1.5f;
    for (int i = 0; i < 4; i++) {
        float shelfY = 0.55f + i * 0.65f;
        FillShelfRow("books_island_s" + std::to_string(i),
                     {cx, shelfY, cz}, 1.5f, 1.7f);
    }
}

// =============================================================================
// BuildTables — 3 reading tables
// =============================================================================
void Scene::BuildTables() {
    auto BuildTable = [&](const std::string& prefix, glm::vec3 center) {
        // Tabletop
        Add(prefix + "_top",  {center.x, 0.78f, center.z},     {2.2f, 0.10f, 1.1f},   WOOD_TABLE);

        // 4 legs
        float lx = 0.9f, lz = 0.4f;
        Add(prefix + "_leg0", {center.x - lx, 0.38f, center.z - lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add(prefix + "_leg1", {center.x + lx, 0.38f, center.z - lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add(prefix + "_leg2", {center.x - lx, 0.38f, center.z + lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
        Add(prefix + "_leg3", {center.x + lx, 0.38f, center.z + lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);

        // X-cross support braces
        Add(prefix + "_brace_a", {center.x, 0.12f, center.z}, {2.0f, 0.05f, 0.07f}, WOOD_MEDIUM);
        Add(prefix + "_brace_b", {center.x, 0.12f, center.z}, {0.07f, 0.05f, 0.9f}, WOOD_MEDIUM);
    };

    BuildTable("table_0", {0.5f, 0.0f,  2.5f});    // Front-center
    BuildTable("table_1", {1.5f, 0.0f,  0.5f});    // Mid-right
    BuildTable("table_2", {2.5f, 0.0f, -1.5f});    // Back-right
}

// =============================================================================
// BuildChairs — Blue-cushioned wooden chairs around each table
// =============================================================================
void Scene::BuildChairs() {
    auto BuildChair = [&](const std::string& prefix, glm::vec3 pos, float yawDeg) {
        (void)yawDeg; // rotation stored for reference, using offsets for simplicity

        // Seat frame (wood)
        Add(prefix + "_seat_frame", pos + glm::vec3(0, 0.48f, 0),
            {0.50f, 0.05f, 0.50f}, WOOD_MEDIUM);

        // Seat cushion (blue)
        Add(prefix + "_cushion", pos + glm::vec3(0, 0.52f, 0),
            {0.44f, 0.06f, 0.44f}, CUSHION_BLUE);

        // Backrest frame
        glm::vec3 backPos = pos + glm::vec3(0, 0.85f, -0.22f);
        Add(prefix + "_back_frame", backPos, {0.50f, 0.60f, 0.05f}, WOOD_MEDIUM);

        // Backrest cushion
        Add(prefix + "_back_cushion", backPos + glm::vec3(0, 0, 0.03f),
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

    // Chairs around table_0 (center: 0.5, 0, 2.5)
    BuildChair("chair_t0_front", { 0.5f, 0.0f, 4.0f},  180.0f);
    BuildChair("chair_t0_back",  { 0.5f, 0.0f, 1.2f},    0.0f);
    BuildChair("chair_t0_left",  {-0.8f, 0.0f, 2.5f},   90.0f);
    BuildChair("chair_t0_right", { 1.8f, 0.0f, 2.5f},  270.0f);

    // Chairs around table_1 (center: 1.5, 0, 0.5)
    BuildChair("chair_t1_front", { 1.5f, 0.0f, 2.0f},  180.0f);
    BuildChair("chair_t1_back",  { 1.5f, 0.0f, -0.8f},   0.0f);
    BuildChair("chair_t1_left",  { 0.2f, 0.0f, 0.5f},   90.0f);
    BuildChair("chair_t1_right", { 2.8f, 0.0f, 0.5f},  270.0f);

    // Chairs around table_2 (center: 2.5, 0, -1.5)
    BuildChair("chair_t2_front", { 2.5f, 0.0f, 0.0f},  180.0f);
    BuildChair("chair_t2_back",  { 2.5f, 0.0f, -2.8f},   0.0f);
    BuildChair("chair_t2_left",  { 1.2f, 0.0f, -1.5f},  90.0f);
    BuildChair("chair_t2_right", { 3.8f, 0.0f, -1.5f}, 270.0f);
}

// =============================================================================
// BuildCeiling — 2 ceiling fans + pendant light clusters
// =============================================================================
void Scene::BuildCeiling() {
    auto BuildFan = [&](const std::string& prefix, glm::vec3 center) {
        // Central hub
        Add(prefix + "_hub", center, {0.20f, 0.15f, 0.20f}, METAL_DARK);

        // Motor housing
        Add(prefix + "_motor", center + glm::vec3(0, -0.12f, 0),
            {0.30f, 0.20f, 0.30f}, METAL_DARK);

        // 5 blades radiating outward
        // TODO Phase 9: Fan blade transforms will be modified each frame by fanAngle rotation
        for (int i = 0; i < 5; i++) {
            float angleDeg = static_cast<float>(i) * 72.0f;
            float angleRad = glm::radians(angleDeg);
            glm::vec3 bladePos = center + glm::vec3(
                cos(angleRad) * 0.65f,
                -0.10f,
                sin(angleRad) * 0.65f
            );
            glm::mat4 bladeModel = Transform::TRS(
                bladePos,
                glm::vec3(0.0f, angleDeg, 8.0f),   // 8° tilt
                glm::vec3(0.70f, 0.04f, 0.20f)      // Long flat blade
            );
            m_Objects.push_back({bladeModel, WOOD_MEDIUM,
                                 prefix + "_blade" + std::to_string(i)});
        }

        // Pendant light hanging below fan
        Add(prefix + "_rod",   center + glm::vec3(0, -0.40f, 0),   {0.04f, 0.50f, 0.04f}, METAL_DARK);
        Add(prefix + "_shade", center + glm::vec3(0, -0.70f, 0),   {0.35f, 0.20f, 0.35f}, METAL_DARK);
    };

    BuildFan("fan_A", {-2.0f, 4.35f,  0.0f});
    BuildFan("fan_B", { 2.5f, 4.35f, -2.0f});

    // Additional pendant clusters (3 in a row, hanging from ceiling)
    for (int i = 0; i < 3; i++) {
        float px = -4.0f + i * 4.0f;
        Add("pendant_rod_" + std::to_string(i),
            {px, 4.0f, -3.5f}, {0.04f, 0.8f, 0.04f}, METAL_DARK);
        Add("pendant_shade_" + std::to_string(i),
            {px, 3.55f, -3.5f}, {0.30f, 0.18f, 0.30f}, METAL_DARK);
    }
}
