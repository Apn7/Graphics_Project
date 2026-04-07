# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Instructions

### Visual Studio (Windows)
Open `3D_Library.slnx` in Visual Studio 2022+, select **Debug | x64**, then build and run (F5). The working directory must be the project root so shaders in `shaders/` are found.

### CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
./3DLibrary      # Linux/macOS
./3DLibrary.exe  # Windows
```

### Dependencies
Place in `external/` before building:
- `external/glad/` — GLAD OpenGL 3.3 Core loader (`glad.h`, `glad.c`, `KHR/khrplatform.h`)
- `external/glm/` — GLM math library
- `external/stb/stb_image.h` — image loader
- `external/glfw/` — GLFW 3.3+ (CMake will try system install first)

## No Automated Tests
Testing is manual/visual. Run the executable and verify rendering using the keyboard controls printed to console on startup.

## Architecture

### Data Flow
```
main()
  → Window + GLAD init
  → ShaderLibrary::LoadAll()    // compiles 4 GLSL programs
  → TextureManager::LoadAll()   // loads 4 textures via stb_image
  → Scene::Build()              // constructs 600+ SceneObjects
  → Render loop:
      InputHandler::ProcessContinuousInput()
      Scene::UpdateAnimations() // fan rotation
      Scene::SetLighting()      // uploads Phong uniforms to shaders
      Scene::Render()           // multi-shader dispatch per object
      Window::SwapBuffers()
```

### Core Subsystems

| Subsystem | Files | Role |
|-----------|-------|------|
| Window | `src/core/Window.h/cpp` | GLFW wrapper, depth test, framebuffer resize |
| Input | `src/core/InputHandler.h/cpp` | GLFW callbacks, continuous input polling, mouse capture (Tab) |
| Camera | `src/core/Camera.h/cpp` | FPS camera: WASD+QE movement, mouse look, FOV zoom |
| Shaders | `src/core/Shader.h/cpp` + `ShaderLibrary.h/cpp` | Compile/link GLSL, uniform caching, singleton registry |
| Mesh | `src/renderer/Mesh.h/cpp` | VAO/VBO/EBO abstraction; Vertex = pos3 + normal3 + UV2 |
| Primitives | `src/renderer/Primitives.h/cpp` | CreateCube, CreatePlane, CreateSphere, CreateCone, CreateBezierVase |
| Textures | `src/renderer/Texture.h/cpp` + `TextureManager.h` | stb_image loading, singleton registry |
| Scene | `src/scene/Scene.h/cpp` | Builds library room from SceneObjects; multi-shader render dispatch |
| Lighting | `src/scene/LightState.h` | Phong uniforms: 1 directional + 3 point lights, day/night presets |

### Shaders (GLSL 3.3, in `shaders/`)

| Shader | Mode |
|--------|------|
| `basic.vert/frag` | Phong lighting on flat color |
| `texture_simple.vert/frag` | Texture only |
| `texture_vertex_blend.vert/frag` | Texture × color at vertex stage |
| `texture_fragment_blend.vert/frag` | Texture × color at fragment stage |

`SceneObject` carries a `TextureMode` enum that determines which shader handles it. A global override (set by `1`–`5` keys) can force all objects to one mode.

### Scene Object Model
`SceneObject` (`src/scene/SceneObject.h`) is a lightweight struct: transform matrix, color, label, texture ID, texture mode, and UV tiling. `Scene::Build()` constructs 600+ of these (room shell, bookshelves, books, tables, chairs, windows, door, fan, lamps) plus 3 curved objects (sphere, cone, Bezier vase).

### Key Constants
- Room bounds: X ∈ [-8, 8], Z ∈ [-7, 7], Y ∈ [0, 6]
- Named colors in `src/scene/LibraryColors.h`
- Transform helpers (TRS, TS, T) in `src/utils/Transform.h`

## Remaining Work (Assignment Requirements)
- Fractal tree/leaves (4/10 marks — mandatory)
- Spot light
- Door/window open animations
- 4-viewport split view
- Custom `glm::rotate` reimplementation
