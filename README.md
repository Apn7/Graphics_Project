# 🏛️ 3D Library Simulation

A 3D Library environment built with **Modern OpenGL 3.3 Core Profile** and **C++17**. The project is structured as a multi-phase build, starting from basic rendering and progressively adding lighting, textures, physics, and more.

---

## 📦 Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| **OpenGL** | 3.3+ | Graphics API |
| **GLFW** | 3.3+ | Window management & input |
| **GLAD** | OpenGL 3.3 Core | OpenGL function loader |
| **GLM** | 0.9.9+ | Math library (vectors, matrices) |
| **stb_image** | Latest | Image loading (Phase 7+) |

---

## 🔧 Build Instructions

### Prerequisites

Place external dependencies in the `external/` folder:
- **GLAD**: Download from [glad.dav1d.de](https://glad.dav1d.de/) (OpenGL 3.3, Core, C/C++) → place at `external/glad/glad.h`, `external/glad/glad.c`, `external/KHR/khrplatform.h`
- **GLM**: Clone from [github.com/g-truc/glm](https://github.com/g-truc/glm) → place at `external/glm/`
- **stb_image**: Download `stb_image.h` → place at `external/stb/stb_image.h`
- **GLFW**: Install system-wide or place in `external/glfw/`

### Windows (Visual Studio / MSVC)

1. Open `3D_Library.slnx` in Visual Studio
2. Make sure the external dependencies are placed correctly
3. Select **Debug | x64** configuration
4. Build → Run (F5)

### Windows / Linux (CMake)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./3DLibrary          # Linux
3DLibrary.exe        # Windows
```

---

## ▶️ How to Run

After building, run the executable from the **project root directory** (not the build folder) so shader files can be found:

```bash
cd 3DLibrary/
./build/3DLibrary
```

---

## 📋 Phase Status

| Phase | Description | Status |
|-------|-------------|--------|
| **Phase 1** | Project Setup & Boilerplate | ✅ Complete |
| **Phase 2** | 3D Room / Walls / Floor | ⬜ Pending |
| **Phase 3** | Furniture & Object Placement | ⬜ Pending |
| **Phase 4** | FPS Camera & Movement | ⬜ Pending |
| **Phase 5** | Scene Graph & Object Management | ⬜ Pending |
| **Phase 6** | Lighting (Phong) | ⬜ Pending |
| **Phase 7** | Textures & Materials | ⬜ Pending |
| **Phase 8** | Advanced Features | ⬜ Pending |
| **Phase 9** | UI / HUD | ⬜ Pending |
| **Phase 10** | Polish & Optimization | ⬜ Pending |

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| `ESC` | Quit (closes window) |
| _More controls coming in Phase 4_ | |

---

## 📁 Project Structure

```
3DLibrary/
├── CMakeLists.txt              # CMake build system
├── README.md                   # This file
├── src/
│   ├── main.cpp                # Entry point — render loop
│   ├── core/
│   │   ├── Window.h/cpp        # GLFW window wrapper
│   │   ├── Shader.h/cpp        # Shader program manager
│   │   └── Camera.h/cpp        # Camera (stub in Phase 1)
│   ├── renderer/
│   │   ├── Mesh.h/cpp          # VAO/VBO/EBO abstraction
│   │   └── Renderer.h          # Static render utilities
│   └── utils/
│       ├── Logger.h            # Console logger (INFO/WARN/ERROR)
│       └── FileUtils.h         # File reading utility
├── shaders/
│   ├── basic.vert              # Vertex shader
│   └── basic.frag              # Fragment shader
├── textures/                   # (Phase 7)
├── assets/                     # (Future models/data)
└── external/                   # Third-party libraries
    ├── glad/                   # OpenGL loader
    ├── KHR/                    # Khronos platform headers
    ├── glm/                    # Math library
    └── stb/                    # Image loader
```
