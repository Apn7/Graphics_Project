# 🏛️ 3D Library Simulation (OpenGL 3.3, C++17)

A production-ready 3D library scene built using Modern OpenGL with a modular C++ architecture.  
The current implementation includes interactive first-person navigation, multi-shader texture pipelines, animated scene elements, and debug visualization modes.

---

## ✨ Implemented Features

### Core Rendering
- Modern OpenGL 3.3 Core Profile pipeline
- Shared cube mesh architecture (scene is composed via transforms)
- Depth testing, real-time rendering loop, dynamic window title telemetry (FPS/object count/modes)

### Scene Content
- Complete indoor library environment:
  - Room shell (floor, ceiling, walls)
  - Windows and door
  - Multiple wall shelf units
  - Books across shelf rows
  - Tables and chairs
  - Ceiling fan assembly
- Scene built from labeled `SceneObject` records with transform/color/texture metadata

### Camera & Interaction
- FPS camera with mouse look and scroll zoom
- Smooth continuous movement with delta-time:
  - `W/A/S/D` movement
  - `Q/E` vertical fly
  - `Left Shift` sprint
- `Tab` toggles cursor capture (FPS mode ↔ free cursor)

### Texture System (Phase 6)
- Per-object texture assignments and UV tiling
- Four rendering modes:
  - Flat color
  - Simple texture
  - Vertex-stage texture/color blend
  - Fragment-stage texture/color blend
- Global texture override for rapid visual testing (`1-5`, `T`)
- Texture registry singleton (`TextureManager`) for load-once/reuse workflow

### Debug & Visualization
- Group-based render filters (`F1-F4`)
- Polygon debug toggles:
  - Wireframe (`F5`)
  - Point cloud (`F6`)
- Animated fan blades updated per frame

---

## 🧱 Technical Implementation

- **Language/Standard:** C++17
- **Graphics API:** OpenGL 3.3 Core
- **Window/Input:** GLFW callbacks + per-frame polling (`InputHandler`)
- **Math:** GLM vectors/matrices
- **OpenGL Loader:** GLAD
- **Image Loading:** `stb_image` (compiled in `src/renderer/Texture.cpp`)

### Architecture Highlights

- **`Scene`** owns all scene objects and animation state, builds object groups, and dispatches multi-shader rendering.
- **`ShaderLibrary`** centralizes shader loading and retrieval (basic + 3 texture shaders).
- **`InputHandler`** encapsulates callbacks and frame input processing to keep `main.cpp` clean.
- **`TextureManager`** provides keyed texture caching and startup loading.
- **`Camera`** handles FPS motion, yaw/pitch vectors, and FOV zoom.

---

## 🎮 Controls

| Category | Key | Action |
|---|---|---|
| Movement | `W / S` | Forward / Backward |
| Movement | `A / D` | Strafe Left / Right |
| Movement | `Q / E` | Fly Up / Down |
| Movement | `Left Shift` | Sprint (hold) |
| Camera | `Mouse` | Look Around |
| Camera | `Scroll` | Zoom (FOV) |
| Camera | `Tab` | Toggle Cursor Capture |
| View Modes | `F1` | Full Scene |
| View Modes | `F2` | Room Shell Only |
| View Modes | `F3` | Furniture Only |
| View Modes | `F4` | Shelves + Books Only |
| Debug | `F5` | Toggle Wireframe |
| Debug | `F6` | Toggle Point Cloud |
| Texture Override | `1` | Per-object mode (default) |
| Texture Override | `2` | Global flat color |
| Texture Override | `3` | Global simple texture |
| Texture Override | `4` | Global vertex blend |
| Texture Override | `5` | Global fragment blend |
| Texture Override | `T` | Cycle texture override modes |
| General | `ESC` | Exit |

---

## 🗂️ Codebase Index

```text
Graphics_Project/
├── CMakeLists.txt
├── 3D_Library.slnx
├── README.md
├── Phase6_TextureMapping.md
├── shaders/
│   ├── basic.vert
│   ├── basic.frag
│   ├── texture_simple.vert
│   ├── texture_simple.frag
│   ├── texture_vertex_blend.vert
│   ├── texture_vertex_blend.frag
│   ├── texture_fragment_blend.vert
│   └── texture_fragment_blend.frag
├── textures/
│   ├── floor_tiles.jpg
│   ├── wall_plaster.jpg
│   ├── wood_dark.jpg
│   └── ceiling.jpg
└── src/
    ├── main.cpp
    ├── core/
    │   ├── Window.h/.cpp
    │   ├── Shader.h/.cpp
    │   ├── ShaderLibrary.h/.cpp
    │   ├── Camera.h/.cpp
    │   └── InputHandler.h/.cpp
    ├── renderer/
    │   ├── Mesh.h/.cpp
    │   ├── Primitives.h/.cpp
    │   ├── Renderer.h
    │   ├── Texture.h/.cpp
    │   └── TextureManager.h
    ├── scene/
    │   ├── Scene.h/.cpp
    │   ├── SceneObject.h
    │   ├── TextureMode.h
    │   └── LibraryColors.h
    └── utils/
        ├── Logger.h
        ├── FileUtils.h
        ├── Transform.h
        └── stb_image.h
```

---

## 📦 Dependencies

| Library | Version | Purpose |
|---|---|---|
| OpenGL | 3.3+ | Graphics API |
| GLFW | 3.3+ | Windowing and input |
| GLAD | OpenGL 3.3 Core | OpenGL function loading |
| GLM | 0.9.9+ | Matrix/vector math |
| stb_image | bundled header | Texture image loading |

---

## 🔧 Build & Run

### Prerequisites

Ensure these are available:
- `external/glad/glad.c` and headers under `external/glad` + `external/KHR`
- `external/glm/` headers
- GLFW 3.3+ (system package or local `external/glfw/`)
- OpenGL development libraries for your platform

### CMake (Linux/macOS/Windows)

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

Run from project root so relative asset paths resolve:

```bash
cd /path/to/Graphics_Project
./build/3DLibrary        # Linux/macOS
build/Debug/3DLibrary.exe  # Windows (multi-config generators like Visual Studio)
build/3DLibrary.exe        # Windows (single-config generators like Ninja/MinGW)
```

### Visual Studio (Windows)

1. Open `3D_Library.slnx`
2. Select `Debug | x64`
3. Build and run (F5)

---

## ✅ Current Status

Phases implemented in code: **Phase 1 through Phase 6** (setup, scene construction, camera/input, scene organization, texture mapping modes, animation/debug controls).  
Future phases (advanced lighting systems, UI/HUD, physics, optimization) are not yet fully implemented.
