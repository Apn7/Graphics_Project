# 🚀 AGENT PROMPT — Phase 6: Texture Mapping
## 3D Library Simulation | OpenGL 3.3 | C++
## Assignment: Textured Objects + Keyboard Toggle Features

---

## 📌 PROJECT CONTEXT

This is **Phase 6** of the 3D Library Simulation. Phases 1–5 are complete. The project has:
- Working first-person camera (WASD + mouse look)
- Full library scene built from transformed cubes: room shell, bookshelves, tables, chairs, ceiling fans
- `Scene` class managing all `SceneObject`s (each has `Transform`, `Color`, `Label`)
- `basic.vert` / `basic.frag` — flat color shader (position + normal + UV layout already set)
- `Mesh` class with correct VAO/VBO/EBO — Vertex struct has `Position`, `Normal`, `TexCoord`
- `Transform::TRS()` for model matrix construction
- `InputHandler` managing GLFW callbacks
- `LibraryColors` namespace with all named colors
- `stb_image.h` already present at `external/stb/stb_image.h`

**Phase 6 goal:** Implement a complete, assignment-compliant texture mapping system covering:
1. **Simple texture** — texture only, no surface color modification (floor)
2. **Blended texture with vertex color** — texture × color computed per-vertex (walls)
3. **Blended texture with fragment color** — texture × color computed per-fragment (table)
4. **Keyboard toggles** to switch between texture modes live

---

## 📁 TEXTURE FILES — WHERE TO GET THEM AND WHERE TO PUT THEM

### Step 1: Download These Free Textures

Download the following from **https://polyhaven.com** (100% free, CC0 license, no attribution needed):

| Texture | What to search | File to download |
|---|---|---|
| Floor tiles | Search: "tiles floor" → pick "Marble Floor Tiles 01" | Download: `2K` JPG, `Diffuse` map |
| Wall plaster | Search: "plaster wall" → pick "Plaster Wall 01" | Download: `2K` JPG, `Diffuse` map |
| Wood dark | Search: "wood floor dark" → pick "Dark Wood Floor 01" | Download: `2K` JPG, `Diffuse` map |

Alternative free source: **https://www.freepbr.com** or **https://ambientcg.com**

**Rename the downloaded files exactly as:**
```
floor_tiles.jpg
wall_plaster.jpg
wood_dark.jpg
```

### Step 2: Where to Put Them in Your Project

Place ALL texture files in:
```
3DLibrary/
└── textures/
    ├── floor_tiles.jpg      ← for the floor (simple texture, no color blend)
    ├── wall_plaster.jpg     ← for walls (texture + vertex color blend)
    └── wood_dark.jpg        ← for table (texture + fragment color blend)
```

### Step 3: Visual Studio — How to Include Them

Textures are **NOT compiled into the executable** — they're loaded at runtime from disk. So in Visual Studio you do NOT add them to the project in Solution Explorer. Instead:

1. Open your project's **property pages** → `Configuration Properties` → `Debugging`
2. Set **Working Directory** to: `$(ProjectDir)` (or wherever your `textures/` folder lives relative to your `.sln` file)
3. Verify the path works by checking that `textures/floor_tiles.jpg` is accessible relative to where the `.exe` runs

**If textures don't load:** The most common mistake is the working directory being set to `$(OutDir)` (the Debug/Release folder) instead of the project root. Fix it in the Debugging property page.

---

## 📁 FILES TO CREATE OR MODIFY

### NEW FILES:
```
src/renderer/
├── Texture.h           # OpenGL texture wrapper
├── Texture.cpp
└── TextureManager.h    # Registry — load once, reuse everywhere

shaders/
├── texture_simple.vert       # Simple texture shader (no color blend)
├── texture_simple.frag
├── texture_vertex_blend.vert # Blend: color computed per-vertex
├── texture_vertex_blend.frag
├── texture_fragment_blend.vert # Blend: color computed per-fragment
└── texture_fragment_blend.frag

src/scene/
└── TextureMode.h       # Enum for toggling modes
```

### MODIFIED FILES:
```
src/renderer/Mesh.h/.cpp      # Ensure UV tiling scale uniform is supported
src/scene/Scene.h/.cpp        # Add texture + mode assignment per object
src/scene/SceneObject.h       # Add TextureMode and texture ID fields
src/core/InputHandler.cpp     # Add T key (cycle texture mode) + number keys
src/main.cpp                  # Load textures, upload to shaders, pass mode
```

---

## 🔧 DETAILED SPECIFICATIONS

---

### `src/scene/TextureMode.h`

```cpp
#pragma once

// Controls how a textured object is rendered.
// Assignment requirement: demonstrate all 3 modes + untextured fallback.
enum class TextureMode {
    FLAT_COLOR      = 0,  // No texture — original Phase 5 flat color (fallback)
    SIMPLE_TEXTURE  = 1,  // Texture only — no surface color modification
    VERTEX_BLEND    = 2,  // Texture × color computed at vertex stage
    FRAGMENT_BLEND  = 3,  // Texture × color computed at fragment stage
};
```

---

### Updated `src/scene/SceneObject.h`

```cpp
#pragma once
#include <glm/glm.hpp>
#include <string>
#include "TextureMode.h"

struct SceneObject {
    glm::mat4   Transform;
    glm::vec3   Color;          // Surface color (used in VERTEX_BLEND and FRAGMENT_BLEND)
    std::string Label;

    // Texture fields (new in Phase 6):
    unsigned int TextureID   = 0;           // OpenGL texture ID (0 = no texture)
    TextureMode  Mode        = TextureMode::FLAT_COLOR;
    float        UVTileX     = 1.0f;        // How many times texture repeats in X
    float        UVTileY     = 1.0f;        // How many times texture repeats in Y
    float        BlendFactor = 0.5f;        // 0.0 = full texture, 1.0 = full color
                                             // Used in VERTEX_BLEND and FRAGMENT_BLEND
};
```

---

### `src/renderer/Texture.h` + `Texture.cpp`

Complete OpenGL texture wrapper using `stb_image`:

```cpp
#pragma once
#include <string>

class Texture {
public:
    // Load from file. Flips image vertically (OpenGL expects bottom-left origin).
    // wrapS/wrapT: GL_REPEAT (default), GL_CLAMP_TO_EDGE, etc.
    // filter: GL_LINEAR (default) or GL_NEAREST
    Texture(const std::string& filepath,
            int wrapMode   = 0x2901,   // GL_REPEAT
            int filterMode = 0x2601);  // GL_LINEAR

    ~Texture();

    // Prevent copy, allow move
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    // Bind to a texture unit (0, 1, 2, ...)
    void Bind(unsigned int unit = 0) const;
    void Unbind() const;

    unsigned int GetID()     const { return m_ID; }
    int          GetWidth()  const { return m_Width; }
    int          GetHeight() const { return m_Height; }
    bool         IsLoaded()  const { return m_ID != 0; }

private:
    unsigned int m_ID     = 0;
    int          m_Width  = 0;
    int          m_Height = 0;
    int          m_Channels = 0;
};
```

**`Texture.cpp` implementation:**
```cpp
// In constructor:
// 1. #define STB_IMAGE_IMPLEMENTATION must appear ONCE in the project.
//    Put it in Texture.cpp ONLY:
//    #define STB_IMAGE_IMPLEMENTATION
//    #include "../../external/stb/stb_image.h"

// 2. stbi_set_flip_vertically_on_load(true);

// 3. unsigned char* data = stbi_load(filepath.c_str(),
//                                     &m_Width, &m_Height, &m_Channels, 0);

// 4. If data is null: LOG_ERROR("Texture failed to load: " + filepath); return;

// 5. Determine format:
//    GLenum format = (m_Channels == 4) ? GL_RGBA :
//                    (m_Channels == 3) ? GL_RGB  : GL_RED;

// 6. glGenTextures(1, &m_ID);
//    glBindTexture(GL_TEXTURE_2D, m_ID);
//    glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0,
//                 format, GL_UNSIGNED_BYTE, data);
//    glGenerateMipmap(GL_TEXTURE_2D);

// 7. Set wrapping and filtering:
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

// 8. stbi_image_free(data);
// 9. glBindTexture(GL_TEXTURE_2D, 0);

// LOG_INFO("Texture loaded: " + filepath +
//          " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");

// Bind():
//    glActiveTexture(GL_TEXTURE0 + unit);
//    glBindTexture(GL_TEXTURE_2D, m_ID);

// Destructor:
//    if (m_ID) glDeleteTextures(1, &m_ID);
```

---

### `src/renderer/TextureManager.h`

```cpp
#pragma once
#include "Texture.h"
#include <unordered_map>
#include <memory>
#include <string>

class TextureManager {
public:
    static TextureManager& Get();

    // Load a texture and store under a key. Returns the GL texture ID.
    // If already loaded under that key, returns existing ID (no reload).
    unsigned int Load(const std::string& key, const std::string& filepath);

    // Get texture object by key (for binding)
    Texture& GetTexture(const std::string& key);

    // Bind a texture by key to a slot
    void Bind(const std::string& key, unsigned int slot = 0);

    // Returns 0 (white fallback) if key not found
    unsigned int GetID(const std::string& key) const;

    // Load all project textures at startup
    void LoadAll();

private:
    TextureManager() = default;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
};
```

**`LoadAll()` implementation:**
```cpp
void TextureManager::LoadAll() {
    Load("floor",  "textures/floor_tiles.jpg");
    Load("wall",   "textures/wall_plaster.jpg");
    Load("wood",   "textures/wood_dark.jpg");

    // TODO Phase 7: Load("book_spine",   "textures/book_spine.jpg");
    // TODO Phase 7: Load("ceiling_wood", "textures/ceiling_wood.jpg");

    LOG_INFO("TextureManager: All textures loaded.");
}
```

---

## 🖼️ THE THREE SHADERS — ASSIGNMENT COMPLIANCE

These three shader pairs directly satisfy the assignment requirements. Each shader pair must be heavily commented explaining WHAT it does and WHY.

---

### Shader 1: Simple Texture (No Color Blend)
**Assignment: "simple texture without surface color"**
**Used by: Floor**

**`shaders/texture_simple.vert`:**
```glsl
#version 330 core

// ============================================================
// SIMPLE TEXTURE VERTEX SHADER
// Assignment: "simple texture without surface color"
// No color calculation here — just pass UV to fragment shader.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

// UV tiling: repeat texture N times across the surface
// e.g., u_TileX=4, u_TileY=4 tiles the texture 4x4 on the floor
uniform float u_TileX;
uniform float u_TileY;

void main() {
    v_FragPos   = vec3(u_Model * vec4(aPos, 1.0));
    v_Normal    = mat3(transpose(inverse(u_Model))) * aNormal;

    // Apply tiling multiplier so texture repeats instead of stretching
    v_TexCoord  = aTexCoord * vec2(u_TileX, u_TileY);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
```

**`shaders/texture_simple.frag`:**
```glsl
#version 330 core

// ============================================================
// SIMPLE TEXTURE FRAGMENT SHADER
// Assignment: "simple texture without surface color"
// Outputs ONLY the texture sample — zero color influence.
// This is the purest form of texturing.
// ============================================================

in vec2 v_TexCoord;
in vec3 v_FragPos;   // Available for Phase 7 lighting — unused now
in vec3 v_Normal;    // Available for Phase 7 lighting — unused now

out vec4 FragColor;

uniform sampler2D u_Texture;   // The bound texture unit

void main() {
    // Sample the texture at the interpolated UV coordinate.
    // No color blending, no surface color — pure texture output.
    vec4 texColor = texture(u_Texture, v_TexCoord);

    FragColor = texColor;

    // TODO Phase 7 (Lighting): FragColor = vec4(ApplyPhong(texColor.rgb, ...), 1.0);
}
```

---

### Shader 2: Texture + Vertex Color Blend
**Assignment: "blended texture with surface color — color computed on the vertex"**
**Used by: Walls**

**`shaders/texture_vertex_blend.vert`:**
```glsl
#version 330 core

// ============================================================
// VERTEX-BLEND TEXTURE VERTEX SHADER
// Assignment: "blended texture with surface color — color on vertex"
// KEY CONCEPT: The blend between texture and surface color is
// computed HERE in the vertex shader. The blended color is
// passed to the fragment shader as a varying.
//
// This means the blend interpolates across the triangle surface.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Output: NOT just UV — we pass the ALREADY-BLENDED color to the fragment
out vec2  v_TexCoord;
out vec3  v_BlendedColor;   // ← This is the assignment's "color computed on vertex"
out vec3  v_FragPos;
out vec3  v_Normal;

uniform mat4  u_Model;
uniform mat4  u_View;
uniform mat4  u_Projection;
uniform float u_TileX;
uniform float u_TileY;

// Surface color (wall cream color — set from C++ per object)
uniform vec3  u_SurfaceColor;

// How much of the surface color to mix in (0.0 = all texture, 1.0 = all color)
uniform float u_BlendFactor;

void main() {
    v_FragPos  = vec3(u_Model * vec4(aPos, 1.0));
    v_Normal   = mat3(transpose(inverse(u_Model))) * aNormal;
    v_TexCoord = aTexCoord * vec2(u_TileX, u_TileY);

    // *** VERTEX-STAGE COLOR BLEND ***
    // We cannot sample the texture here (no sampler in vertex shaders).
    // So we pre-compute the surface color contribution at the vertex.
    // The fragment shader will multiply this by the texture sample.
    //
    // v_BlendedColor represents: "how much of the surface color does this
    // vertex contribute to the final blend?"
    //
    // Mix formula: result = texture_color * (1 - blend) + surface_color * blend
    // But since we can't sample texture here, we pass:
    //   v_BlendedColor = mix(vec3(1.0), u_SurfaceColor, u_BlendFactor)
    // Then in fragment: FragColor = texture * v_BlendedColor
    //
    // When blend = 0.0: v_BlendedColor = (1,1,1) → texture × 1 = pure texture
    // When blend = 1.0: v_BlendedColor = u_SurfaceColor → texture tinted fully
    v_BlendedColor = mix(vec3(1.0), u_SurfaceColor, u_BlendFactor);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
```

**`shaders/texture_vertex_blend.frag`:**
```glsl
#version 330 core

// ============================================================
// VERTEX-BLEND TEXTURE FRAGMENT SHADER
// The blending was already computed in the vertex shader.
// This shader just applies the pre-computed blend to the texture.
// ============================================================

in vec2  v_TexCoord;
in vec3  v_BlendedColor;   // Received from vertex shader (interpolated across triangle)
in vec3  v_FragPos;
in vec3  v_Normal;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    // Sample the texture
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // Apply the vertex-computed blend color.
    // Since v_BlendedColor was computed per-vertex and interpolated,
    // this is technically a vertex-stage color operation applied in the fragment.
    vec3 finalColor = texColor.rgb * v_BlendedColor;

    FragColor = vec4(finalColor, texColor.a);

    // TODO Phase 7: Apply Phong lighting to finalColor before output
}
```

---

### Shader 3: Texture + Fragment Color Blend
**Assignment: "blended texture with surface color — color computed on the fragment"**
**Used by: Table**

**`shaders/texture_fragment_blend.vert`:**
```glsl
#version 330 core

// ============================================================
// FRAGMENT-BLEND TEXTURE VERTEX SHADER
// Assignment: "blended texture with surface color — color on fragment"
// KEY CONCEPT: Unlike the vertex blend shader, NO color computation
// happens here. We only pass raw data to the fragment shader.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 v_TexCoord;
out vec3 v_FragPos;    // World position (for fragment-stage calculations)
out vec3 v_Normal;

uniform mat4  u_Model;
uniform mat4  u_View;
uniform mat4  u_Projection;
uniform float u_TileX;
uniform float u_TileY;

void main() {
    v_FragPos  = vec3(u_Model * vec4(aPos, 1.0));
    v_Normal   = mat3(transpose(inverse(u_Model))) * aNormal;
    v_TexCoord = aTexCoord * vec2(u_TileX, u_TileY);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);

    // NOTE: No color computation here — that is the whole point of this shader.
    // The difference from texture_vertex_blend: blend factor applied in fragment.
}
```

**`shaders/texture_fragment_blend.frag`:**
```glsl
#version 330 core

// ============================================================
// FRAGMENT-BLEND TEXTURE FRAGMENT SHADER
// Assignment: "blended texture with surface color — color on fragment"
//
// KEY DIFFERENCE from vertex blend:
// - Vertex blend: blend factor applied at vertex stage, interpolated
// - Fragment blend: blend factor applied fresh per-fragment using
//   fragment's world position, normal, or a uniform
//
// This allows per-fragment effects like:
// - Distance-based color fade
// - Normal-dependent tinting
// - Procedural color variation across the surface
// ============================================================

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3      u_SurfaceColor;   // Table dark wood color
uniform float     u_BlendFactor;    // 0.0 = texture only, 1.0 = color only

void main() {
    // Sample texture
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // *** FRAGMENT-STAGE COLOR BLEND ***
    // Unlike vertex blend, the mix() is computed HERE for EVERY fragment.
    // This means every pixel gets its own fresh blend calculation.
    //
    // For the table: we use a simple uniform blend factor.
    // But this could be made more complex per-fragment, for example:
    //   float dynamicBlend = abs(v_Normal.y) * u_BlendFactor;
    // to tint top faces more than side faces (demonstrates fragment power).

    // Surface color contribution computed per-fragment:
    vec3 surfaceContrib = u_SurfaceColor * u_BlendFactor;

    // Texture contribution:
    vec3 textureContrib = texColor.rgb * (1.0 - u_BlendFactor);

    // Final blend — computed entirely in fragment stage:
    vec3 finalColor = textureContrib + surfaceContrib;

    FragColor = vec4(finalColor, texColor.a);

    // TODO Phase 7: Multiply finalColor by Phong lighting result
}
```

---

## 🔧 UPDATED `Scene.cpp` — Texture Assignment

In `Scene::Build()`, after all objects are constructed, assign textures and modes:

```cpp
void Scene::AssignTextures() {
    // Get texture IDs from manager
    unsigned int floorTex = TextureManager::Get().GetID("floor");
    unsigned int wallTex  = TextureManager::Get().GetID("wall");
    unsigned int woodTex  = TextureManager::Get().GetID("wood");

    for (auto& obj : m_Objects) {

        // FLOOR — Simple texture, no color blend
        if (obj.Label == "floor") {
            obj.TextureID   = floorTex;
            obj.Mode        = TextureMode::SIMPLE_TEXTURE;
            obj.UVTileX     = 6.0f;   // Tile 6 times across 12-unit floor
            obj.UVTileY     = 5.0f;   // Tile 5 times across 10-unit depth
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
                 obj.Label.find("top")  != std::string::npos) {
            obj.TextureID   = woodTex;
            obj.Mode        = TextureMode::FRAGMENT_BLEND;
            obj.UVTileX     = 2.0f;
            obj.UVTileY     = 1.0f;
            obj.Color       = LibraryColors::WOOD_TABLE;
            obj.BlendFactor = 0.40f;
        }

        // Everything else remains FLAT_COLOR (Phase 5 behavior unchanged)
    }
}
```

Call `AssignTextures()` at the end of `Build()`.

---

## 🎮 KEYBOARD TOGGLES — Assignment Requirement

**`src/scene/TextureMode.h`** — Add a global state:

```cpp
// Global texture mode override.
// When set, ALL objects render in this mode regardless of their assigned mode.
// When NONE (-1), each object uses its own assigned mode.
enum class GlobalTextureOverride {
    NONE            = -1,  // Each object uses its own assigned mode
    ALL_FLAT        = 0,   // Override: everything renders flat color (Phase 5 look)
    ALL_SIMPLE      = 1,   // Override: everything uses simple texture
    ALL_VERTEX      = 2,   // Override: everything uses vertex blend
    ALL_FRAGMENT    = 3,   // Override: everything uses fragment blend
};
```

**Keyboard bindings (add to `InputHandler::KeyCallback`):**

```
T           — Cycle through GlobalTextureOverride values (NONE → ALL_FLAT → ALL_SIMPLE → ALL_VERTEX → ALL_FRAGMENT → NONE)
1           — Set GlobalTextureOverride::NONE (each object uses its own mode — default)
2           — Set GlobalTextureOverride::ALL_FLAT (no textures, flat color only)
3           — Set GlobalTextureOverride::ALL_SIMPLE (all objects: simple texture)
4           — Set GlobalTextureOverride::ALL_VERTEX (all objects: vertex blend)
5           — Set GlobalTextureOverride::ALL_FRAGMENT (all objects: fragment blend)
```

**Window title update** — append current mode to title string:
```cpp
std::string modeStr[] = {"PER-OBJECT", "ALL-FLAT", "ALL-SIMPLE",
                          "ALL-VERTEX", "ALL-FRAGMENT"};
// Add to the existing FPS/position title:
title += " | Texture: " + modeStr[(int)g_GlobalOverride + 1];
```

---

## 🔧 SCENE RENDER — UPDATED `Scene::Render()`

The render function must select the correct shader per object based on mode:

```cpp
void Scene::Render(Shader& flatShader,
                   Shader& simpleShader,
                   Shader& vertexBlendShader,
                   Shader& fragmentBlendShader,
                   GlobalTextureOverride override)
{
    for (const auto& obj : m_Objects) {

        // Determine effective mode
        TextureMode effectiveMode = obj.Mode;
        if (override != GlobalTextureOverride::NONE)
            effectiveMode = static_cast<TextureMode>((int)override);

        // Select and use the right shader
        Shader* shader = nullptr;
        switch (effectiveMode) {
            case TextureMode::SIMPLE_TEXTURE:  shader = &simpleShader;        break;
            case TextureMode::VERTEX_BLEND:    shader = &vertexBlendShader;   break;
            case TextureMode::FRAGMENT_BLEND:  shader = &fragmentBlendShader; break;
            default:                           shader = &flatShader;          break;
        }

        shader->Use();

        // Upload common uniforms
        shader->SetMat4("u_Model",      obj.Transform);
        shader->SetMat4("u_View",       m_View);      // stored from last SetCamera() call
        shader->SetMat4("u_Projection", m_Projection);

        // Upload texture-specific uniforms
        if (effectiveMode != TextureMode::FLAT_COLOR) {
            shader->SetFloat("u_TileX", obj.UVTileX);
            shader->SetFloat("u_TileY", obj.UVTileY);
            shader->SetInt("u_Texture", 0);  // Texture unit 0

            // Bind the object's texture (or fallback to 1x1 white if none)
            if (obj.TextureID != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.TextureID);
            }
        }

        // Color blend uniforms
        if (effectiveMode == TextureMode::VERTEX_BLEND ||
            effectiveMode == TextureMode::FRAGMENT_BLEND) {
            shader->SetVec3("u_SurfaceColor", obj.Color);
            shader->SetFloat("u_BlendFactor",  obj.BlendFactor);
        }

        // Flat color fallback (original behavior)
        if (effectiveMode == TextureMode::FLAT_COLOR) {
            shader->SetVec3("u_Color",  obj.Color);
            shader->SetFloat("u_Alpha", 1.0f);
        }

        m_CubeMesh->Draw();
    }
}
```

Also add:
```cpp
void Scene::SetCameraMatrices(const glm::mat4& view, const glm::mat4& projection) {
    m_View       = view;
    m_Projection = projection;
}
```

And add `m_View`, `m_Projection` as private members of `Scene`.

---

## 🔧 UPDATED `ShaderLibrary`

Add the 3 new texture shaders:

```cpp
void ShaderLibrary::LoadAll() {
    Load("basic",            "shaders/basic.vert",                "shaders/basic.frag");
    Load("texture_simple",   "shaders/texture_simple.vert",       "shaders/texture_simple.frag");
    Load("texture_vertex",   "shaders/texture_vertex_blend.vert",  "shaders/texture_vertex_blend.frag");
    Load("texture_fragment", "shaders/texture_fragment_blend.vert","shaders/texture_fragment_blend.frag");

    // TODO Phase 7 (Lighting): Load("phong", "shaders/phong.vert", "shaders/phong.frag");

    LOG_INFO("ShaderLibrary: All shaders loaded.");
}

Shader& ShaderLibrary::GetTextureSimple()   { return *m_Shaders.at("texture_simple"); }
Shader& ShaderLibrary::GetTextureVertex()   { return *m_Shaders.at("texture_vertex"); }
Shader& ShaderLibrary::GetTextureFragment() { return *m_Shaders.at("texture_fragment"); }
```

---

## 🔧 UPDATED `main.cpp`

```cpp
// Startup order:
TextureManager::Get().LoadAll();      // Load all textures from disk
ShaderLibrary::Get().LoadAll();       // Compile all shaders
scene.Build();                        // Build geometry + assign textures/modes

// Global state:
GlobalTextureOverride g_Override = GlobalTextureOverride::NONE;

// Pass g_Override to InputHandler so key callbacks can modify it.
InputHandler::Init(window.GetNativeWindow(), &camera, &g_Override);

// Render loop:
scene.SetCameraMatrices(camera.GetViewMatrix(), projection);
scene.Render(
    ShaderLibrary::Get().GetBasic(),
    ShaderLibrary::Get().GetTextureSimple(),
    ShaderLibrary::Get().GetTextureVertex(),
    ShaderLibrary::Get().GetTextureFragment(),
    g_Override
);
```

---

## ✅ ACCEPTANCE CRITERIA

- [ ] Floor renders with **tile texture only** — no color tinting visible (SIMPLE_TEXTURE)
- [ ] Walls render with **plaster texture tinted cream** — subtle wall color visible (VERTEX_BLEND)
- [ ] Table tops render with **wood texture + dark tint** — clearly different from walls (FRAGMENT_BLEND)
- [ ] Pressing `2` removes all textures — scene looks like Phase 5 (flat colors)
- [ ] Pressing `3` applies simple texture to ALL objects
- [ ] Pressing `4` applies vertex blend to ALL objects
- [ ] Pressing `5` applies fragment blend to ALL objects
- [ ] Pressing `1` or `T` cycling returns to per-object mode
- [ ] Window title shows current texture mode name
- [ ] Floor texture tiles correctly — no stretched single tile
- [ ] `LOG_ERROR` fires if any texture file is not found at the expected path
- [ ] `LOG_INFO` confirms each texture loaded with dimensions
- [ ] `stb_image` `#define STB_IMAGE_IMPLEMENTATION` appears exactly ONCE (in `Texture.cpp`)
- [ ] All shelves, chairs, books, ceiling, fans remain flat-colored (Phase 5 behavior, untouched)

---

## 🚫 STRICT RULES

1. **`#define STB_IMAGE_IMPLEMENTATION` in `Texture.cpp` only** — in any other file causes duplicate symbol linker errors
2. **`stbi_set_flip_vertically_on_load(true)` must be called** before any `stbi_load` — without this textures appear upside down
3. **UV tiling via uniform** — never hardcode tiling in the shader
4. **`glActiveTexture(GL_TEXTURE0)` before `glBindTexture`** — always
5. **Texture IDs stored in `SceneObject`** — not looked up every frame from TextureManager
6. **`GL_REPEAT` wrap mode** — so tiled textures work correctly (not `GL_CLAMP_TO_EDGE`)
7. **`GL_LINEAR_MIPMAP_LINEAR` for minification** — prevents aliasing on the tiled floor
8. **Assignment compliance comments in every shader** — each shader's header comment must name the assignment requirement it satisfies

---

## 🔮 FUTURE PHASE HOOKS

- In every texture shader frag file:
  `// TODO Phase 7 (Lighting): Multiply final color by Phong lighting result`
- In `SceneObject.h`:
  `// TODO Phase 7: Add Material struct for ambient/diffuse/specular/shininess`
- In `TextureManager::LoadAll()`:
  `// TODO Phase 7: Load book_spine.jpg, ceiling_wood.jpg for full scene texturing`
- In `Scene::AssignTextures()`:
  `// TODO Phase 7: Assign textures to bookshelves, books, ceiling, chair frames`
- In `texture_fragment_blend.frag`:
  `// TODO Phase 7: Replace u_BlendFactor with dynamic lighting-based blend`

---

**Build order:**
`TextureMode.h` → Update `SceneObject.h` → `Texture.h/cpp` → `TextureManager.h/cpp` → 3 shader pairs (6 files) → `ShaderLibrary` update → `Scene::AssignTextures()` → `Scene::Render()` update → `InputHandler` key bindings → `main.cpp` update → verify all 5 keyboard modes work.
