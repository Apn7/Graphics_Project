# 🚀 AGENT PROMPT — Phase 5: Scene Construction (Room + Furniture)
## 3D Library Simulation | OpenGL 3.3 | C++

---

## 📌 PROJECT CONTEXT

This is **Phase 5** of a multi-phase 3D Library Simulation in Modern OpenGL 3.3 + C++.

**Phases 1–4 are complete.** The project has:
- Working window, GLAD, deltaTime render loop
- Fully robust `Shader` class + `basic.vert`/`basic.frag` (flat color, no lighting yet)
- `Mesh` class with correct VAO/VBO/EBO and Vertex struct (Position + Normal + TexCoord)
- `Primitives::CreateCube()` and `CreatePlane()` — correct normals, 24 verts/cube
- `Transform::TRS(position, rotationDeg, scale)` — clean model matrix builder
- Fully working first-person `Camera` with mouse look + WASD + sprint
- `InputHandler` managing all GLFW callbacks
- `ShaderLibrary` singleton

**Phase 5 goal:** Build the **complete library scene** using transformed cubes. Every object — walls, floor, ceiling, bookshelves, tables, chairs, ceiling fans — is constructed from `Primitives::CreateCube()` with different `Transform::TRS` values and different `u_Color` values. No lighting yet. Just geometry in the right place. After this phase, you should be able to walk through a fully furnished 3D library.

---

## 🏛️ REFERENCE SCENE (Your Library — Study This Carefully)

Your target library has:
- **Room shell:** cream/white walls, light gray tiled floor, dark coffered wooden ceiling
- **Left wall:** 2 tall full-height bookshelves side by side (very tall, goes almost to ceiling)
- **Back wall:** 3 medium freestanding shelf units in a row
- **Center-left:** 1 freestanding shelf island (double-sided, accessible from both sides)
- **Right wall:** partial shelf + wooden door
- **Tables:** 3–4 dark oval/rounded reading tables (simulate oval with a scaled cube for now)
- **Chairs:** 4–6 blue-cushioned wooden chairs around each table
- **Ceiling:** Coffered dark wood panels, 2 ceiling fans, pendant light clusters hanging down
- **Windows:** 2 tall arched windows (left and right walls) — teal/cyan colored quads

**Room coordinate system:**
- **Width (X):** -6 to +6 (12 units wide)
- **Depth (Z):** -5 to +5 (10 units deep)
- **Height (Y):** 0 (floor) to 4.5 (ceiling)
- **Camera starts at:** (0, 1.7, 5) — near front wall, looking toward -Z

---

## 🎯 WHAT TO BUILD IN PHASE 5

---

### 1. `SceneObject` — `src/scene/SceneObject.h`

A lightweight struct that holds everything needed to draw one piece of geometry:

```cpp
#pragma once
#include <glm/glm.hpp>
#include <string>

struct SceneObject {
    glm::mat4  Transform;   // Pre-computed model matrix (computed once, not every frame)
    glm::vec3  Color;       // Flat color (used until Phase 6 replaces with Material)
    std::string Label;      // Human-readable name for debugging ("left_wall", "table_1_top")
};
```

---

### 2. `Scene` Class — `src/scene/Scene.h` + `Scene.cpp`

Manages the entire collection of scene objects. Separates scene data from the render loop.

```cpp
#pragma once
#include "SceneObject.h"
#include <vector>
#include "../core/Shader.h"
#include "../renderer/Mesh.h"

class Scene {
public:
    Scene();
    ~Scene() = default;

    // Build the entire library. Called once at startup.
    void Build();

    // Draw all objects. Shader must already be Use()'d before calling.
    // Sets u_Model and u_Color per object, then calls cubeMesh->Draw().
    void Render(Shader& shader);

    // Optional: draw only a named group (for debugging)
    // e.g., RenderGroup("shelf") draws all objects whose Label contains "shelf"
    void RenderGroup(Shader& shader, const std::string& groupPrefix);

private:
    // The single cube mesh used for everything
    std::unique_ptr<Mesh> m_CubeMesh;

    // All scene objects — walls, furniture, books, everything
    std::vector<SceneObject> m_Objects;

    // Helper: adds a SceneObject to m_Objects
    void Add(const std::string& label,
             const glm::vec3& position,
             const glm::vec3& rotDeg,
             const glm::vec3& scale,
             const glm::vec3& color);

    // Shorthand for axis-aligned objects (no rotation)
    void Add(const std::string& label,
             const glm::vec3& position,
             const glm::vec3& scale,
             const glm::vec3& color);

    // --- Group builders (called by Build()) ---
    void BuildRoom();        // Floor, ceiling, 4 walls
    void BuildWindows();     // Arched window quads
    void BuildDoor();        // Wooden door on right wall
    void BuildWallShelves(); // Left wall tall shelves + back wall shelves
    void BuildIslandShelf(); // Center freestanding shelf unit
    void BuildTables();      // All reading tables
    void BuildChairs();      // All chairs
    void BuildCeiling();     // Coffered panels + fan hubs + blade positions
    void BuildBooks();       // Colorful books filling shelf rows
};
```

**`Scene::Render()` implementation:**
```cpp
void Scene::Render(Shader& shader) {
    for (const auto& obj : m_Objects) {
        shader.SetMat4("u_Model", obj.Transform);
        shader.SetVec3("u_Color", obj.Color);
        m_CubeMesh->Draw();
    }
}
```

---

### 3. Color Constants — `src/scene/LibraryColors.h`

Header-only. Name every color used in the scene — essential for maintainability and for explaining to your professor what each object is:

```cpp
#pragma once
#include <glm/glm.hpp>

namespace LibraryColors {
    // Structural
    constexpr glm::vec3 WALL_CREAM      = {0.92f, 0.90f, 0.85f};
    constexpr glm::vec3 FLOOR_TILE      = {0.78f, 0.78f, 0.80f};
    constexpr glm::vec3 CEILING_WOOD    = {0.22f, 0.14f, 0.06f};

    // Wood (shelves, tables, door, chair frames)
    constexpr glm::vec3 WOOD_DARK       = {0.30f, 0.18f, 0.06f};  // Dark walnut shelves
    constexpr glm::vec3 WOOD_MEDIUM     = {0.45f, 0.28f, 0.10f};  // Chair frames, door
    constexpr glm::vec3 WOOD_TABLE      = {0.25f, 0.14f, 0.05f};  // Dark reading tables

    // Chair cushions
    constexpr glm::vec3 CUSHION_BLUE    = {0.12f, 0.18f, 0.55f};  // Navy blue
    constexpr glm::vec3 CUSHION_DARK    = {0.08f, 0.12f, 0.40f};  // Cushion shadow areas

    // Windows
    constexpr glm::vec3 WINDOW_TEAL     = {0.40f, 0.82f, 0.80f};  // Teal/cyan glass
    constexpr glm::vec3 WINDOW_FRAME    = {0.22f, 0.14f, 0.06f};  // Dark wood frame

    // Books — varied colors for visual interest
    constexpr glm::vec3 BOOK_RED        = {0.75f, 0.15f, 0.10f};
    constexpr glm::vec3 BOOK_BLUE       = {0.15f, 0.30f, 0.70f};
    constexpr glm::vec3 BOOK_GREEN      = {0.10f, 0.55f, 0.20f};
    constexpr glm::vec3 BOOK_YELLOW     = {0.85f, 0.75f, 0.10f};
    constexpr glm::vec3 BOOK_ORANGE     = {0.85f, 0.40f, 0.10f};
    constexpr glm::vec3 BOOK_PURPLE     = {0.45f, 0.10f, 0.65f};
    constexpr glm::vec3 BOOK_WHITE      = {0.90f, 0.88f, 0.85f};
    constexpr glm::vec3 BOOK_BROWN      = {0.50f, 0.28f, 0.10f};

    // Metal/lamp
    constexpr glm::vec3 METAL_DARK      = {0.15f, 0.15f, 0.15f};  // Fan fixtures
    constexpr glm::vec3 LAMP_WARM       = {1.00f, 0.92f, 0.60f};  // Pendant bulb color
}
```

---

### 4. Room Shell — `Scene::BuildRoom()`

The outer box of the library. All faces point **inward** (camera is inside):

```
FLOOR:
  Label: "floor"
  Position: (0, 0, 0)
  Scale: (12, 0.2, 10)      ← thin slab covering full room footprint
  Color: FLOOR_TILE

CEILING:
  Label: "ceiling"
  Position: (0, 4.5, 0)
  Scale: (12, 0.2, 10)
  Color: CEILING_WOOD

LEFT WALL (X = -6):
  Label: "wall_left"
  Position: (-6, 2.25, 0)
  Scale: (0.2, 4.5, 10)
  Color: WALL_CREAM

RIGHT WALL (X = +6):
  Label: "wall_right"
  Position: (6, 2.25, 0)
  Scale: (0.2, 4.5, 10)
  Color: WALL_CREAM

BACK WALL (Z = -5):
  Label: "wall_back"
  Position: (0, 2.25, -5)
  Scale: (12, 4.5, 0.2)
  Color: WALL_CREAM

FRONT WALL (Z = +5):
  Label: "wall_front"
  Position: (0, 2.25, 5)
  Scale: (12, 4.5, 0.2)
  Color: WALL_CREAM
```

---

### 5. Windows — `Scene::BuildWindows()`

Two tall arched windows. Simulate the arch by stacking two quads (rectangle + smaller square on top):

```
LEFT WALL WINDOW (primary):
  Rect portion:
    Label: "window_left_rect"
    Position: (-5.9, 2.0, -1.0)       ← on left wall, toward back
    Scale: (0.1, 2.5, 1.8)
    Color: WINDOW_TEAL

  Arch cap (smaller, on top):
    Label: "window_left_arch"
    Position: (-5.9, 3.4, -1.0)
    Scale: (0.1, 0.8, 1.2)
    Color: WINDOW_TEAL

  Window frame (dark surround):
    Label: "window_left_frame"
    Position: (-5.92, 2.2, -1.0)
    Scale: (0.05, 3.0, 2.2)
    Color: WINDOW_FRAME

RIGHT WALL WINDOW:
  Mirror of left window at X = +5.9, same Z position
```

---

### 6. Door — `Scene::BuildDoor()`

A tall wooden door on the right wall:

```
Door panel:
  Label: "door"
  Position: (5.9, 1.8, 2.5)
  Scale: (0.15, 3.6, 1.8)
  Color: WOOD_MEDIUM

Door frame (surround):
  Label: "door_frame"
  Position: (5.9, 1.9, 2.5)
  Scale: (0.1, 4.0, 2.2)
  Color: WOOD_DARK

Door handle (small cube):
  Label: "door_handle"
  Position: (5.8, 1.8, 1.7)
  Scale: (0.15, 0.08, 0.08)
  Color: METAL_DARK
```

---

### 7. Wall Bookshelves — `Scene::BuildWallShelves()`

#### Left Wall — 2 Tall Shelf Units Side by Side

Each shelf unit is made of: 2 side panels + 1 back panel + 5–6 horizontal shelves + top panel + bottom base.

**Helper lambda inside BuildWallShelves (define locally):**
```cpp
// Builds one complete tall shelf unit at given X offset on the left wall
auto BuildTallShelf = [&](float zCenter, const std::string& prefix) {
    float x = -5.5f;  // Hugging left wall

    // Back panel
    Add(prefix+"_back",   {x, 2.0f, zCenter}, {0.1f, 4.0f, 1.8f}, WOOD_DARK);
    // Left side panel
    Add(prefix+"_left",   {x+0.85f, 2.0f, zCenter-0.85f}, {0.1f, 4.0f, 0.1f}, WOOD_DARK);
    // Right side panel
    Add(prefix+"_right",  {x+0.85f, 2.0f, zCenter+0.85f}, {0.1f, 4.0f, 0.1f}, WOOD_DARK);
    // Top panel
    Add(prefix+"_top",    {x+0.45f, 4.0f, zCenter}, {0.9f, 0.08f, 1.8f}, WOOD_DARK);
    // Base
    Add(prefix+"_base",   {x+0.45f, 0.1f, zCenter}, {0.9f, 0.15f, 1.8f}, WOOD_DARK);
    // 5 shelves at even vertical intervals
    for (int i = 0; i < 5; i++) {
        float shelfY = 0.6f + i * 0.65f;
        Add(prefix+"_shelf"+std::to_string(i),
            {x+0.45f, shelfY, zCenter},
            {0.9f, 0.06f, 1.8f}, WOOD_DARK);
    }
};

BuildTallShelf(-2.0f, "shelf_wall_left_A");   // Left of the two units
BuildTallShelf( 0.5f, "shelf_wall_left_B");   // Right of the two units
```

#### Back Wall — 3 Medium Shelf Units

```cpp
auto BuildMediumShelf = [&](float xCenter, const std::string& prefix) {
    float z = -4.7f;  // Hugging back wall

    Add(prefix+"_back",  {xCenter, 1.8f, z},         {1.6f, 3.2f, 0.1f}, WOOD_DARK);
    Add(prefix+"_left",  {xCenter-0.75f, 1.8f, z+0.3f}, {0.08f, 3.2f, 0.6f}, WOOD_DARK);
    Add(prefix+"_right", {xCenter+0.75f, 1.8f, z+0.3f}, {0.08f, 3.2f, 0.6f}, WOOD_DARK);
    Add(prefix+"_top",   {xCenter, 3.3f, z+0.3f},    {1.6f, 0.07f, 0.6f}, WOOD_DARK);
    Add(prefix+"_base",  {xCenter, 0.1f, z+0.3f},    {1.6f, 0.12f, 0.6f}, WOOD_DARK);
    for (int i = 0; i < 4; i++) {
        float shelfY = 0.55f + i * 0.65f;
        Add(prefix+"_shelf"+std::to_string(i),
            {xCenter, shelfY, z+0.3f},
            {1.6f, 0.06f, 0.6f}, WOOD_DARK);
    }
};

BuildMediumShelf(-3.5f, "shelf_back_A");
BuildMediumShelf( 0.0f, "shelf_back_B");
BuildMediumShelf( 3.5f, "shelf_back_C");
```

---

### 8. Island Shelf — `Scene::BuildIslandShelf()`

One freestanding shelf unit in the center-left of the room, accessible from both sides:

```cpp
float cx = -2.5f, cz = -1.5f;

// Frame
Add("island_left",   {cx-0.75f, 1.8f, cz}, {0.08f, 3.2f, 1.8f}, WOOD_DARK);
Add("island_right",  {cx+0.75f, 1.8f, cz}, {0.08f, 3.2f, 1.8f}, WOOD_DARK);
Add("island_top",    {cx, 3.3f, cz},        {1.6f, 0.07f, 1.8f}, WOOD_DARK);
Add("island_base",   {cx, 0.1f, cz},        {1.6f, 0.12f, 1.8f}, WOOD_DARK);
// Shelves
for (int i = 0; i < 4; i++) {
    Add("island_shelf"+std::to_string(i),
        {cx, 0.55f + i * 0.65f, cz},
        {1.6f, 0.06f, 1.8f}, WOOD_DARK);
}
```

---

### 9. Books — `Scene::BuildBooks()`

Fill every shelf with tightly packed colorful books. Books are thin tall cuboids.

**Book color rotation array:**
```cpp
const glm::vec3 bookColors[] = {
    BOOK_RED, BOOK_BLUE, BOOK_GREEN, BOOK_YELLOW,
    BOOK_ORANGE, BOOK_PURPLE, BOOK_WHITE, BOOK_BROWN
};
int colorCount = 8;
```

**For each shelf row, generate books like this:**
```cpp
// Example: fill one shelf row
// shelfPos = center of shelf surface (top face of the shelf plank)
// shelfWidth = total usable width, shelfDepth = depth of shelf
auto FillShelfRow = [&](const std::string& prefix,
                         glm::vec3 shelfPos, float shelfWidth, float shelfDepth) {
    float bookW  = 0.07f;   // Spine width
    float bookH  = 0.50f;   // Book height
    float bookD  = shelfDepth * 0.85f;   // Nearly full depth

    int count    = (int)(shelfWidth / (bookW + 0.01f));
    float startX = shelfPos.x - shelfWidth * 0.5f + bookW * 0.5f;

    for (int i = 0; i < count; i++) {
        glm::vec3 bPos = {
            startX + i * (bookW + 0.01f),
            shelfPos.y + bookH * 0.5f + 0.03f,  // Sitting on shelf surface
            shelfPos.z
        };
        glm::vec3 bColor = bookColors[i % colorCount];
        Add(prefix + "_book" + std::to_string(i),
            bPos, {bookW, bookH, bookD}, bColor);
    }
};
```

Call `FillShelfRow` for every shelf in every bookshelf unit. This should produce hundreds of colorful book spines visible throughout the room — the visual centrepiece of the library.

---

### 10. Reading Tables — `Scene::BuildTables()`

3 dark reading tables arranged in a diagonal row from left-front to right-back:

```cpp
// Helper: one reading table = thick top + 4 legs + optional X-brace underneath
auto BuildTable = [&](const std::string& prefix, glm::vec3 center) {
    // Tabletop
    Add(prefix+"_top",  {center.x, 0.78f, center.z}, {2.2f, 0.10f, 1.1f}, WOOD_TABLE);

    // 4 legs
    float lx = 0.9f, lz = 0.4f;
    Add(prefix+"_leg0", {center.x-lx, 0.38f, center.z-lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
    Add(prefix+"_leg1", {center.x+lx, 0.38f, center.z-lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
    Add(prefix+"_leg2", {center.x-lx, 0.38f, center.z+lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);
    Add(prefix+"_leg3", {center.x+lx, 0.38f, center.z+lz}, {0.08f, 0.78f, 0.08f}, WOOD_MEDIUM);

    // X-cross support brace (visible from under table — matches reference image)
    Add(prefix+"_brace_a", center + glm::vec3(0, 0.12f, 0),
        {2.0f, 0.05f, 0.07f}, WOOD_MEDIUM);
    Add(prefix+"_brace_b", center + glm::vec3(0, 0.12f, 0),
        {0.07f, 0.05f, 0.9f}, WOOD_MEDIUM);
};

BuildTable("table_0", { 0.5f, 0.0f,  2.5f});   // Front-center
BuildTable("table_1", { 1.5f, 0.0f,  0.5f});   // Mid-right
BuildTable("table_2", { 2.5f, 0.0f, -1.5f});   // Back-right
```

---

### 11. Chairs — `Scene::BuildChairs()`

6 blue-cushioned chairs around each table. Each chair = seat + cushion + backrest + 4 legs + 2 armrests:

```cpp
auto BuildChair = [&](const std::string& prefix, glm::vec3 pos, float yawDeg) {
    // Apply yaw rotation around Y for chair facing direction
    // (Use Transform::TRS with rotation for the seat and back)

    // Seat frame (wood)
    Add(prefix+"_seat_frame", pos + glm::vec3(0, 0.48f, 0),
        {0.50f, 0.05f, 0.50f}, WOOD_MEDIUM);

    // Seat cushion (blue, slightly smaller)
    Add(prefix+"_cushion", pos + glm::vec3(0, 0.52f, 0),
        {0.44f, 0.06f, 0.44f}, CUSHION_BLUE);

    // Backrest frame
    glm::vec3 backPos = pos + glm::vec3(0, 0.85f, -0.22f);
    Add(prefix+"_back_frame", backPos, {0.50f, 0.60f, 0.05f}, WOOD_MEDIUM);

    // Backrest cushion
    Add(prefix+"_back_cushion", backPos + glm::vec3(0, 0, 0.03f),
        {0.44f, 0.54f, 0.04f}, CUSHION_BLUE);

    // 4 legs
    float lx = 0.20f, lz = 0.20f, legH = 0.48f;
    Add(prefix+"_leg0", pos + glm::vec3(-lx, legH*0.5f, -lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
    Add(prefix+"_leg1", pos + glm::vec3( lx, legH*0.5f, -lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
    Add(prefix+"_leg2", pos + glm::vec3(-lx, legH*0.5f,  lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);
    Add(prefix+"_leg3", pos + glm::vec3( lx, legH*0.5f,  lz), {0.05f, legH, 0.05f}, WOOD_MEDIUM);

    // Armrests (2)
    Add(prefix+"_arm_l", pos + glm::vec3(-0.27f, 0.65f, 0), {0.04f, 0.04f, 0.48f}, WOOD_MEDIUM);
    Add(prefix+"_arm_r", pos + glm::vec3( 0.27f, 0.65f, 0), {0.04f, 0.04f, 0.48f}, WOOD_MEDIUM);
};

// Place 4 chairs around table_0 (one at each side):
BuildChair("chair_t0_front", { 0.5f, 0, 4.0f},  180.0f);  // Front — facing table
BuildChair("chair_t0_back",  { 0.5f, 0, 1.2f},    0.0f);  // Back
BuildChair("chair_t0_left",  {-0.8f, 0, 2.5f},   90.0f);  // Left
BuildChair("chair_t0_right", { 1.8f, 0, 2.5f},  270.0f);  // Right

// Place 4 chairs around table_1 and table_2 similarly (offset positions)
```

---

### 12. Ceiling Fan — `Scene::BuildCeiling()`

2 ceiling fans + pendant light clusters. Fan blades will be **animated in Phase 9** — for now, just build the geometry with correct labels so animation can target them by name.

```cpp
auto BuildFan = [&](const std::string& prefix, glm::vec3 center) {
    // Central hub (small cylinder approximated as a cube)
    Add(prefix+"_hub", center, {0.20f, 0.15f, 0.20f}, METAL_DARK);

    // Motor housing (below hub)
    Add(prefix+"_motor", center + glm::vec3(0, -0.12f, 0),
        {0.30f, 0.20f, 0.30f}, METAL_DARK);

    // 5 blades radiating outward
    // Blades are thin flat cuboids, rotated 72° apart around Y
    for (int i = 0; i < 5; i++) {
        float angleDeg = i * 72.0f;
        float angleRad = glm::radians(angleDeg);
        glm::vec3 bladePos = center + glm::vec3(
            cos(angleRad) * 0.65f,
            -0.10f,
            sin(angleRad) * 0.65f
        );
        // Store blade transform WITH rotation (so Phase 9 can rotate all blades together)
        // Use TRS with Y-rotation = angleDeg
        glm::mat4 bladeModel = Transform::TRS(
            bladePos,
            glm::vec3(0, angleDeg, 8.0f),   // 8° tilt (like real fan blades)
            glm::vec3(0.70f, 0.04f, 0.20f)  // Long flat blade
        );
        // Add directly with pre-computed matrix:
        m_Objects.push_back({bladeModel, WOOD_MEDIUM,
                             prefix + "_blade" + std::to_string(i)});
    }

    // Pendant light hanging below fan
    // Light rod
    Add(prefix+"_rod", center + glm::vec3(0, -0.40f, 0),
        {0.04f, 0.50f, 0.04f}, METAL_DARK);
    // Lamp shade
    Add(prefix+"_shade", center + glm::vec3(0, -0.70f, 0),
        {0.35f, 0.20f, 0.35f}, METAL_DARK);
};

// 2 fans positioned along center line of room
BuildFan("fan_A", {-2.0f, 4.35f,  0.0f});
BuildFan("fan_B", { 2.5f, 4.35f, -2.0f});

// Additional pendant clusters (3 in a row, hanging from ceiling, no fan)
for (int i = 0; i < 3; i++) {
    float px = -4.0f + i * 4.0f;
    Add("pendant_rod_"+std::to_string(i),
        {px, 4.0f, -3.5f}, {0.04f, 0.8f, 0.04f}, METAL_DARK);
    Add("pendant_shade_"+std::to_string(i),
        {px, 3.55f, -3.5f}, {0.30f, 0.18f, 0.30f}, METAL_DARK);
}
```

---

### 13. Updated `main.cpp`

Replace the Phase 3 test cubes with the Scene system:

```cpp
// Startup:
Scene scene;
scene.Build();

// In render loop:
shader.Use();
shader.SetMat4("u_View",       camera.GetViewMatrix());
shader.SetMat4("u_Projection", projection);
shader.SetFloat("u_Alpha",     1.0f);
scene.Render(shader);
```

That's it. `main.cpp` stays clean — all scene knowledge lives in `Scene`.

---

### 14. Debug Mode — `F1` Key

Wire up a debug render mode that calls `RenderGroup(shader, "shelf")` or similar to isolate parts of the scene. Useful for explaining specific objects to your professor:

```cpp
// In InputHandler KeyCallback:
// F1: render all objects (default)
// F2: render only room shell (walls/floor/ceiling)
// F3: render only furniture (tables + chairs)
// F4: render only shelves + books
```

Add a `static int g_RenderMode = 0;` in main and branch on it before calling `scene.Render`.

---

## ✅ ACCEPTANCE CRITERIA

- [ ] Full room shell renders correctly — can walk inside, walls surround you
- [ ] Two tall left-wall bookshelves visible, packed with colorful books
- [ ] Three back-wall shelf units with books
- [ ] One island shelf unit in center-left
- [ ] Three reading tables with X-brace legs visible underneath
- [ ] Blue-cushioned wooden chairs around each table (at least 3 chairs per table)
- [ ] Two ceiling fans with hub, motor, 5 blades each
- [ ] Pendant light fixtures hanging from ceiling
- [ ] Two arched windows with teal color on left and right walls
- [ ] Wooden door with frame and handle on right wall
- [ ] Camera can walk the full room without passing through walls (walls stop you visually)
- [ ] Window title still shows live FPS and position (from Phase 4)
- [ ] F1–F4 debug render modes work
- [ ] `main.cpp` has NO object construction logic — only `scene.Build()` and `scene.Render()`
- [ ] `LOG_INFO` at startup reports total object count: "Scene built: N objects"

---

## 🚫 STRICT RULES

1. **One `Mesh` instance only** (`m_CubeMesh`) — all objects share it, only the model matrix differs
2. **All positions derived from the coordinate system** (-6 to +6 X, -5 to +5 Z, 0 to 4.5 Y)
3. **All colors from `LibraryColors` namespace** — no raw `glm::vec3(r,g,b)` literals in Build functions
4. **Labels are mandatory** — every `SceneObject` must have a meaningful label
5. **No OpenGL calls in `Scene.cpp`** — only in `Mesh::Draw()`
6. **`BuildFan` blade transforms pre-computed** — stored as `glm::mat4`, not recomputed every frame

---

## 🔮 FUTURE PHASE HOOKS

- In `SceneObject`: `// TODO Phase 6: Replace Color with Material (ambient/diffuse/specular/shininess)`
- In `Scene::Render()`: `// TODO Phase 6: Upload u_Material per object instead of u_Color`
- In `BuildCeiling` fan blades: `// TODO Phase 9: Fan blade transforms will be modified each frame by fanAngle rotation`
- In `Scene.h`: `// TODO Phase 9: Add UpdateAnimations(float deltaTime) method for fan rotation`
- In `InputHandler KeyCallback`: `// TODO Phase 8: L key and N key for lighting`

---

**Build order:**
`LibraryColors.h` → `SceneObject.h` → `Scene.h/cpp` (empty Build stubs) → `BuildRoom()` → verify room renders → `BuildWallShelves()` → `BuildBooks()` → `BuildTables()` → `BuildChairs()` → `BuildCeiling()` → `BuildWindows()` → `BuildDoor()` → `main.cpp` update → debug modes → verify full scene.
