# 🚀 AGENT PROMPT — Phase 3: Basic 3D Primitives (VAO/VBO/EBO)
## 3D Library Simulation | OpenGL 3.3 | C++

---

## 📌 PROJECT CONTEXT

This is **Phase 3** of a multi-phase 3D Library Simulation in Modern OpenGL 3.3 + C++.

**Phases 1 & 2 are complete.** The project has:
- Working window, GLAD, render loop with deltaTime
- Fully robust `Shader` class with uniform caching
- `basic.vert` / `basic.frag` — flat color shader, position + normal + UV layout
- `ShaderLibrary` singleton
- `Logger` and `FileUtils` utilities
- A stub `Mesh` class and a stub `Camera` class

**Phase 3 goal:** Build a rock-solid, reusable **primitive geometry system** centered on a `Cube` — the single building block used to construct every object in the library scene (walls, floor, ceiling, bookshelves, tables, chairs, fan blades, books — all of them are transformed cubes or combinations of cubes).

This phase is about making it **trivially easy** to place, scale, and color a cube anywhere in 3D space. Everything in Phase 5 depends on getting this right.

---

## 🎯 WHAT TO BUILD IN PHASE 3

---

### 1. Updated `Vertex` Struct — `src/renderer/Mesh.h`

This is the single vertex format used for the entire project. Define it once, use it everywhere:

```cpp
struct Vertex {
    glm::vec3 Position;   // Object-space XYZ
    glm::vec3 Normal;     // Outward-facing surface normal (unit vector)
    glm::vec2 TexCoord;   // UV coordinates (0.0–1.0 range)
};
```

**Why these fields now?** The shader already expects them at locations 0, 1, 2. The VAO attribute layout must match the shader or nothing renders. Normals are needed for lighting in Phase 6 — setting them up correctly now avoids a painful refactor later.

---

### 2. Fully Implemented `Mesh` Class — `src/renderer/Mesh.h` + `Mesh.cpp`

The `Mesh` class wraps a VAO/VBO/EBO and draws indexed geometry. It must be clean, reusable, and have zero OpenGL calls leaking outside it.

**`src/renderer/Mesh.h`:**

```cpp
#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};

class Mesh {
public:
    // Upload geometry to GPU. Call once at construction.
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<unsigned int>& indices);

    // Prevent accidental copy (OpenGL objects can't be trivially copied)
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Allow move (so we can store Meshes in vectors)
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    ~Mesh();

    // Bind VAO and issue a draw call
    void Draw() const;

    // Accessors for debug/stats
    unsigned int GetVertexCount() const { return m_VertexCount; }
    unsigned int GetIndexCount()  const { return m_IndexCount;  }

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    unsigned int m_VertexCount = 0;
    unsigned int m_IndexCount  = 0;

    void SetupMesh(const std::vector<Vertex>& vertices,
                   const std::vector<unsigned int>& indices);
    void Release(); // Delete GPU resources (called by destructor and move assignment)
};
```

**`src/renderer/Mesh.cpp` — Implementation requirements:**

`SetupMesh`:
```
glGenVertexArrays(1, &m_VAO)
glGenBuffers(1, &m_VBO)
glGenBuffers(1, &m_EBO)
glBindVertexArray(m_VAO)

glBindBuffer(GL_ARRAY_BUFFER, m_VBO)
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), data, GL_STATIC_DRAW)

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), data, GL_STATIC_DRAW)

// Attribute 0: Position (vec3, offset 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0)
glEnableVertexAttribArray(0)

// Attribute 1: Normal (vec3, offset 12 bytes = offsetof(Vertex, Normal))
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal))
glEnableVertexAttribArray(1)

// Attribute 2: TexCoord (vec2, offset 24 bytes = offsetof(Vertex, TexCoord))
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord))
glEnableVertexAttribArray(2)

glBindVertexArray(0) // Unbind VAO — important!
```

`Draw`:
```
glBindVertexArray(m_VAO)
glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0)
glBindVertexArray(0)
```

`Release`:
```
glDeleteVertexArrays(1, &m_VAO)
glDeleteBuffers(1, &m_VBO)
glDeleteBuffers(1, &m_EBO)
// Reset IDs to 0 so destructor on moved-from object is safe
```

---

### 3. `Primitives` Namespace — `src/renderer/Primitives.h` + `Primitives.cpp`

**This is the most important file in Phase 3.** It's a namespace with factory functions that return ready-to-use vertex and index data for common shapes. Phase 5 will call these constantly.

```cpp
#pragma once
#include "Mesh.h"
#include <memory>

namespace Primitives {

    // Returns a unit cube centered at origin.
    // Size: 1x1x1. Each face has 4 unique vertices with correct outward normals.
    // UV maps 0,0 → 1,1 per face.
    // Use glm::scale(model, glm::vec3(w, h, d)) to resize.
    std::unique_ptr<Mesh> CreateCube();

    // Returns a flat horizontal plane (for floor/ceiling).
    // Lies in XZ plane, centered at origin, size 1x1.
    // Normal points +Y. Scale to desired size.
    std::unique_ptr<Mesh> CreatePlane();

    // Convenience: creates a cube mesh and immediately draws it
    // at a given model transform using the given shader's u_Model uniform.
    // Use this in Phase 5 to reduce boilerplate.
    void DrawCube(class Shader& shader, const glm::mat4& modelMatrix);

} // namespace Primitives
```

---

### 4. Cube Geometry — Full Specification

**CreateCube()** must return a cube with these exact properties:

- Centered at origin `(0, 0, 0)`
- Extends from `(-0.5, -0.5, -0.5)` to `(+0.5, +0.5, +0.5)` — unit cube
- **24 vertices** (4 per face × 6 faces) — faces do NOT share vertices because each face needs its own unique normal vector
- **36 indices** (6 per face × 6 faces = 2 triangles per face)

**The 6 faces and their normals:**

| Face   | Normal          | Vertices (Position)                                                                 |
|--------|-----------------|--------------------------------------------------------------------------------------|
| Front  | `(0, 0, +1)`    | `(-0.5,-0.5, 0.5)`, `(0.5,-0.5, 0.5)`, `(0.5, 0.5, 0.5)`, `(-0.5, 0.5, 0.5)`   |
| Back   | `(0, 0, -1)`    | `(0.5,-0.5,-0.5)`, `(-0.5,-0.5,-0.5)`, `(-0.5, 0.5,-0.5)`, `(0.5, 0.5,-0.5)`   |
| Left   | `(-1, 0, 0)`    | `(-0.5,-0.5,-0.5)`, `(-0.5,-0.5, 0.5)`, `(-0.5, 0.5, 0.5)`, `(-0.5, 0.5,-0.5)` |
| Right  | `(+1, 0, 0)`    | `(0.5,-0.5, 0.5)`, `(0.5,-0.5,-0.5)`, `(0.5, 0.5,-0.5)`, `(0.5, 0.5, 0.5)`    |
| Top    | `(0, +1, 0)`    | `(-0.5, 0.5, 0.5)`, `(0.5, 0.5, 0.5)`, `(0.5, 0.5,-0.5)`, `(-0.5, 0.5,-0.5)`  |
| Bottom | `(0, -1, 0)`    | `(-0.5,-0.5,-0.5)`, `(0.5,-0.5,-0.5)`, `(0.5,-0.5, 0.5)`, `(-0.5,-0.5, 0.5)`  |

**UV coordinates for each face** (same pattern for all 6 faces):
```
Vertex 0 (bottom-left):  UV (0.0, 0.0)
Vertex 1 (bottom-right): UV (1.0, 0.0)
Vertex 2 (top-right):    UV (1.0, 1.0)
Vertex 3 (top-left):     UV (0.0, 1.0)
```

**Index pattern for each face** (two triangles, counter-clockwise winding):
```
Face base index B (where B = face_number * 4):
Triangle 1: B+0, B+1, B+2
Triangle 2: B+2, B+3, B+0
```

---

### 5. Plane Geometry — `CreatePlane()`

A flat quad in the XZ plane, size 1×1, centered at origin, normal pointing `+Y`:

```
4 vertices:
(-0.5, 0.0, -0.5)  UV(0,0)
( 0.5, 0.0, -0.5)  UV(1,0)
( 0.5, 0.0,  0.5)  UV(1,1)
(-0.5, 0.0,  0.5)  UV(0,1)
Normal: (0, 1, 0) for all 4 vertices

6 indices: 0,1,2, 2,3,0
```

---

### 6. `Transform` Helper — `src/utils/Transform.h`

A tiny header-only helper that makes building model matrices clean. Phase 5 will call this hundreds of times.

```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Transform {

    // Build a complete model matrix in one line.
    // Example: Transform::TRS({0,1,0}, {0,45,0}, {2,0.1f,2})
    inline glm::mat4 TRS(
        const glm::vec3& position,
        const glm::vec3& rotationDegrees = glm::vec3(0.0f),
        const glm::vec3& scale           = glm::vec3(1.0f))
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, position);
        m = glm::rotate(m, glm::radians(rotationDegrees.x), glm::vec3(1,0,0));
        m = glm::rotate(m, glm::radians(rotationDegrees.y), glm::vec3(0,1,0));
        m = glm::rotate(m, glm::radians(rotationDegrees.z), glm::vec3(0,0,1));
        m = glm::scale(m, scale);
        return m;
    }

    // Shorthand: position + uniform scale (no rotation)
    inline glm::mat4 TS(const glm::vec3& position, float uniformScale) {
        return TRS(position, glm::vec3(0.0f), glm::vec3(uniformScale));
    }

    // Shorthand: position only (identity rotation and scale)
    inline glm::mat4 T(const glm::vec3& position) {
        return glm::translate(glm::mat4(1.0f), position);
    }

} // namespace Transform
```

---

### 7. Updated `main.cpp` — Phase 3 Demo Scene

Replace the single rotating test cube from Phase 2 with a small demo that proves primitives work:

```cpp
// Setup (before loop):
auto cubeMesh  = Primitives::CreateCube();
auto planeMesh = Primitives::CreatePlane();
Shader& shader = ShaderLibrary::Get().GetBasic();

// Projection matrix (set once, update if window resizes):
glm::mat4 projection = glm::perspective(
    glm::radians(60.0f),               // FOV
    (float)window.GetWidth() / window.GetHeight(), // Aspect ratio
    0.1f,                              // Near clip
    100.0f                             // Far clip
);

// Camera view (static for now — Phase 4 makes this dynamic):
glm::mat4 view = glm::lookAt(
    glm::vec3(4.0f, 3.0f, 6.0f),   // Eye position
    glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
    glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
);

// In render loop — draw 5 cubes in different positions/scales/colors:
shader.Use();
shader.SetMat4("u_Projection", projection);
shader.SetMat4("u_View", view);
shader.SetFloat("u_Alpha", 1.0f);

// Floor plane — scaled large, light gray
shader.SetMat4("u_Model", Transform::TRS({0,0,0}, {0,0,0}, {10,1,8}));
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

// Cube 4 — tall shelf (non-uniform scale demonstration)
shader.SetMat4("u_Model", Transform::TRS({-4.0f, 1.5f, 0.0f}, {0,0,0}, {0.5f, 3.0f, 0.4f}));
shader.SetVec3("u_Color", glm::vec3(0.35f, 0.20f, 0.08f));
cubeMesh->Draw();

// Cube 5 — rotating (confirms deltaTime accumulation still works)
static float angle = 0.0f;
angle += 30.0f * deltaTime;
shader.SetMat4("u_Model", Transform::TRS({4.0f, 0.5f, 0.0f}, {0, angle, 0}));
shader.SetVec3("u_Color", glm::vec3(0.15f, 0.55f, 0.35f));
cubeMesh->Draw();
```

The scene should show: a gray floor, 4 colored cubes (demonstrating different library colors), 1 tall scaled "shelf-like" shape, 1 rotating cube — all lit by flat color but placed correctly in 3D space with depth testing working.

---

## ✅ ACCEPTANCE CRITERIA

- [ ] Five cubes and a floor plane render correctly with different colors
- [ ] Each cube is drawn with the correct color set via `u_Color`
- [ ] Scaling works — the tall cube (`0.5 × 3.0 × 0.4`) looks like a shelf pillar
- [ ] Depth testing is correct — no cubes bleed through the floor
- [ ] The rotating cube spins smoothly (uses `deltaTime` — speed same regardless of FPS)
- [ ] `Transform::TRS` compiles and works correctly
- [ ] `Primitives::CreateCube()` returns a valid mesh with 24 vertices and 36 indices
- [ ] `Primitives::CreatePlane()` returns a valid flat mesh
- [ ] No VAO/VBO/EBO leaks — confirmed by `Release()` being called in destructor
- [ ] Moving the camera eye position (changing the hardcoded `lookAt`) changes perspective correctly

---

## 🚫 STRICT RULES

1. **24 vertices per cube — non-negotiable.** Sharing vertices between faces breaks normals in Phase 6. Don't do it even though 8 would technically render.
2. **`offsetof()` for attribute pointers** — not hardcoded byte counts (breaks if struct changes)
3. **`std::unique_ptr<Mesh>`** for all primitive factories — enforces ownership clarity
4. **No OpenGL calls in `Primitives.h`** — only in `Primitives.cpp`
5. **`Transform::TRS` is the only way to build model matrices** — no raw `glm::translate/rotate/scale` chains in `main.cpp`

---

## 🔮 FUTURE PHASE HOOKS

- In `Primitives.h`: `// TODO Phase 5: Add CreateBookshelf(), CreateChair(), CreateTable(), CreateFan()`
- In `Mesh.cpp SetupMesh`: `// TODO Phase 7: When textures are added, ensure TexCoord UVs tile correctly on large surfaces`
- In `main.cpp` view matrix: `// TODO Phase 4: Replace hardcoded lookAt with Camera::GetViewMatrix()`
- In `Transform.h`: `// TODO Phase 9: Add parent-child transform composition for fan blade animation`

---

**Build order:** `Vertex` struct → `Mesh.h/cpp` → `Primitives.h/cpp` → `Transform.h` → `main.cpp` update → verify all 5 cubes render.
