# 🚀 AGENT PROMPT — Phase 1: Project Setup & Boilerplate
## 3D Library Simulation | OpenGL 3.3 | C++

---

## 🎯 YOUR MISSION

You are setting up the **complete foundational codebase** for a 3D Library Simulation built with Modern OpenGL 3.3 and C++. This is Phase 1 of a multi-phase project. Your job is to create a **production-ready, well-documented, easily extensible** project from scratch — every file should be clean, every line should be understandable, and every system should be built to be expanded in future phases.

**Do NOT cut corners. Do NOT skip comments. Do NOT use deprecated OpenGL.**

---

## 📁 EXACT FILE STRUCTURE TO CREATE

Create the following structure. Every file listed must be created:

```
3DLibrary/
│
├── CMakeLists.txt                  # Build system — cross platform
├── README.md                       # Project overview and build instructions
│
├── src/
│   ├── main.cpp                    # Entry point — window creation, main loop
│   │
│   ├── core/
│   │   ├── Window.h                # GLFW window abstraction
│   │   ├── Window.cpp
│   │   ├── Shader.h                # Shader program loader/compiler
│   │   ├── Shader.cpp
│   │   ├── Camera.h                # FPS-style camera (to be fully implemented Phase 4)
│   │   └── Camera.cpp              # Stub only in Phase 1 — just initial position
│   │
│   ├── renderer/
│   │   ├── Mesh.h                  # VAO/VBO/EBO abstraction
│   │   ├── Mesh.cpp
│   │   └── Renderer.h              # Central render call dispatcher (stub for now)
│   │
│   └── utils/
│       ├── Logger.h                # Simple console logger with levels (INFO, WARN, ERROR)
│       └── FileUtils.h             # Read shader/text files from disk
│
├── shaders/
│   ├── basic.vert                  # Vertex shader — position + color
│   └── basic.frag                  # Fragment shader — solid color output
│
├── textures/                       # Empty folder, ready for Phase 7
│   └── .gitkeep
│
├── assets/                         # For any future models or data files
│   └── .gitkeep
│
└── external/                       # Third-party headers (header-only or submodules)
    ├── glad/                       # GLAD OpenGL loader
    │   ├── glad.h
    │   └── glad.c
    ├── KHR/
    │   └── khrplatform.h
    ├── glm/                        # GLM math library (header only)
    └── stb/
        └── stb_image.h             # Image loader (header only, for Phase 7)
```

---

## 🔧 WHAT TO BUILD IN EACH FILE

### `CMakeLists.txt`
- CMake version 3.16+
- C++17 standard
- Find and link: `OpenGL`, `glfw3`
- Include directories: `src/`, `external/`
- Compile `external/glad/glad.c` as part of sources
- Output executable named `3DLibrary`

---

### `src/utils/Logger.h`
A simple, header-only logger. Must support:
```cpp
LOG_INFO("Window created successfully");
LOG_WARN("Texture not found, using default");
LOG_ERROR("Shader compilation failed");
```
- Use `std::cout` with color codes (ANSI) and timestamps
- Prefix: `[INFO]`, `[WARN]`, `[ERROR]`
- Keep it header-only (no .cpp needed)

---

### `src/utils/FileUtils.h`
Header-only utility:
```cpp
std::string FileUtils::ReadFile(const std::string& filepath);
```
- Opens file, reads entire content as string
- Logs error via Logger if file not found
- Used by Shader class to load `.vert` / `.frag` files

---

### `src/core/Window.h` + `Window.cpp`
A clean GLFW window wrapper. Must:
- Initialize GLFW with OpenGL 3.3 Core Profile hints
- Create a resizable window (default: 1280x720)
- Load GLAD after context creation — fail loudly if it doesn't load
- Set a window resize callback that updates `glViewport`
- Expose:
  ```cpp
  bool ShouldClose();
  void SwapBuffers();
  void PollEvents();
  GLFWwindow* GetNativeWindow();
  int GetWidth(), GetHeight();
  ```
- Clean GLFW terminate in destructor

---

### `src/core/Shader.h` + `Shader.cpp`
Full shader program manager. Must:
- Accept paths to `.vert` and `.frag` files
- Read files using `FileUtils::ReadFile()`
- Compile vertex and fragment shaders separately
- Check for compile errors — print the error log with `LOG_ERROR`
- Link into a program — check for link errors
- Expose:
  ```cpp
  void Use();
  void SetInt(const std::string& name, int value);
  void SetFloat(const std::string& name, float value);
  void SetVec3(const std::string& name, const glm::vec3& value);
  void SetMat4(const std::string& name, const glm::mat4& value);
  unsigned int GetID();
  ```
- Every uniform setter must find the uniform location by name

---

### `src/core/Camera.h` + `Camera.cpp`
**Phase 1 stub only** — don't build the full system yet, but define the class interface cleanly so future phases can expand it:
- Store: `glm::vec3 Position`, `glm::vec3 Front`, `glm::vec3 Up`
- Constructor sets a default position: `(0.0f, 1.5f, 5.0f)` looking toward origin
- Expose:
  ```cpp
  glm::mat4 GetViewMatrix();   // returns glm::lookAt(...)
  glm::vec3 GetPosition();
  // TODO markers for Phase 4: ProcessKeyboard(), ProcessMouseMovement()
  ```

---

### `src/renderer/Mesh.h` + `Mesh.cpp`
Core rendering primitive. Wraps VAO/VBO/EBO. Must:
- Accept a list of vertices (position `vec3` + color `vec3` — normals & UVs will be added in Phase 6/7)
- Accept a list of indices (for EBO)
- Set up VAO, VBO, EBO correctly in constructor
- Expose:
  ```cpp
  void Draw();       // binds VAO and calls glDrawElements
  void Delete();     // cleans up GPU memory
  ```
- Use a proper `Vertex` struct:
  ```cpp
  struct Vertex {
      glm::vec3 Position;
      glm::vec3 Color;
      // glm::vec3 Normal;    // TODO Phase 6
      // glm::vec2 TexCoord;  // TODO Phase 7
  };
  ```

---

### `src/renderer/Renderer.h`
Stub/interface only in Phase 1:
```cpp
class Renderer {
public:
    static void Clear(float r, float g, float b, float a = 1.0f);
    // Future: Draw(Mesh&, Shader&, glm::mat4 transform)
};
```
- Implement `Clear()` to call `glClearColor` + `glClear`

---

### `shaders/basic.vert`
```glsl
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
```

---

### `shaders/basic.frag`
```glsl
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
```

---

### `src/main.cpp`
The entry point. Must demonstrate ALL systems working together:

1. Create `Window` (1280x720, "3D Library - OpenGL 3.3")
2. Enable `glEnable(GL_DEPTH_TEST)`
3. Create `Shader` from `shaders/basic.vert` and `shaders/basic.frag`
4. Create a test `Mesh` — a **colored cube** (8 vertices, 36 indices) with different colors per vertex
5. Create a `Camera` at default position
6. Set up **projection matrix**: `glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f)`
7. **Main loop**:
   - Poll events
   - Clear screen (dark navy: `0.05, 0.07, 0.12`)
   - Calculate `deltaTime` using `glfwGetTime()`
   - Set shader uniforms: model (rotating slowly using `deltaTime`), view, projection
   - Draw the cube
   - Swap buffers
8. Cleanup on exit

**The cube should slowly rotate on the Y-axis** so you can see all faces and confirm everything works.

---

### `README.md`
Include:
- Project description (2-3 lines)
- Dependencies: OpenGL 3.3+, GLFW3, GLM, GLAD
- Build instructions for Windows (MSVC/MinGW) and Linux
- How to run
- Current Phase status: `[Phase 1 ✅] [Phase 2 ⬜] ... [Phase 10 ⬜]`
- Controls table (to be filled each phase)

---

## ✅ ACCEPTANCE CRITERIA — Phase 1 is DONE when:

- [ ] Project builds with zero errors and zero warnings
- [ ] A window opens titled "3D Library - OpenGL 3.3"
- [ ] A colored cube renders in the center of the screen
- [ ] The cube slowly rotates on the Y-axis
- [ ] Background is dark (near black/navy)
- [ ] Closing the window exits cleanly (no crash, no memory leak warnings)
- [ ] All files have header comment blocks explaining their purpose
- [ ] No raw OpenGL calls in `main.cpp` — everything goes through the abstraction classes
- [ ] Every `.cpp` and `.h` file has comments explaining what each block does

---

## 🚫 STRICT RULES — DO NOT VIOLATE

1. **OpenGL 3.3 Core Profile ONLY** — no legacy `glBegin/glEnd`, no compatibility profile
2. **No global variables** — use class members
3. **No magic numbers** — use named constants or pass as parameters
4. **Comment every non-obvious line** — this project must be fully explainable to a professor
5. **Separate concerns** — Window code stays in Window, shader code stays in Shader, etc.
6. **Use `glm::` for all math** — no hand-rolled matrix math
7. **Check every OpenGL/GLFW call** that can fail — log errors, don't silently continue

---

## 🔮 FUTURE PHASE AWARENESS

When writing code, add `// TODO Phase X:` comments wherever future work plugs in. Examples:

- In `Vertex` struct: `// TODO Phase 6: Add Normal and TexCoord`
- In `Camera.cpp`: `// TODO Phase 4: Add keyboard/mouse movement`
- In `Mesh.cpp`: `// TODO Phase 6: Update attribute pointers for normals`
- In `main.cpp`: `// TODO Phase 5: Replace test cube with Scene objects`

This makes it immediately clear WHERE to add new code in every future phase.

---

## 📦 EXTERNAL DEPENDENCIES SETUP NOTE

Assume the developer will manually place:
- GLAD files at `external/glad/glad.h`, `external/glad/glad.c`, `external/KHR/khrplatform.h`  
  (Generated from: https://glad.dav1d.de/ — OpenGL 3.3, Core, C/C++)
- GLM at `external/glm/` (header-only, cloned from https://github.com/g-truc/glm)
- stb_image at `external/stb/stb_image.h`
- GLFW installed system-wide OR placed in `external/glfw/`

CMakeLists should handle both cases gracefully.

---

**Start with `CMakeLists.txt`, then utilities, then core systems, then renderer, then shaders, then `main.cpp` last. Build in this order.**
