# 🚀 AGENT PROMPT — Phase 4: Camera System
## 3D Library Simulation | OpenGL 3.3 | C++

---

## 📌 PROJECT CONTEXT

This is **Phase 4** of a multi-phase 3D Library Simulation in Modern OpenGL 3.3 + C++.

**Phases 1–3 are complete.** The project has:
- Working window, GLAD, render loop with accurate deltaTime
- Fully robust `Shader` class with uniform caching
- `Primitives::CreateCube()` and `CreatePlane()` returning correct VAO/VBO/EBO meshes
- `Transform::TRS()` for clean model matrix construction
- A static `glm::lookAt(...)` view matrix hardcoded in `main.cpp`
- A `Camera` class **stub** with only `GetViewMatrix()` returning a fixed lookAt

**Phase 4 goal:** Replace the static stub camera with a fully interactive **first-person camera** that lets you freely walk through the library scene. After this phase, you should be able to navigate the entire room, inspect every object from any angle, and feel genuinely "inside" the library.

---

## 🎯 WHAT TO BUILD IN PHASE 4

---

### 1. Fully Implemented `Camera` Class

**`src/core/Camera.h`:**

```cpp
#pragma once
#include <glm/glm.hpp>

// Movement direction enum — cleaner than raw key constants in camera code
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,        // Q key — fly up (useful for inspecting scene top-down)
    DOWN       // E key — fly down
};

class Camera {
public:
    // -------------------------------------------------------
    // Construction
    // -------------------------------------------------------
    // Default starting position: inside the library, near the entrance,
    // looking toward the center of the room.
    // Position: (0, 1.7, 5) — standing height, near front wall
    // Yaw: -90 degrees (facing -Z, toward back of room)
    // Pitch: 0 degrees (level horizon)
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 1.7f, 5.0f),
        float yaw          = -90.0f,
        float pitch        = 0.0f
    );

    // -------------------------------------------------------
    // Core Output
    // -------------------------------------------------------

    // Returns the view matrix for this frame. Upload to shader as u_View.
    glm::mat4 GetViewMatrix() const;

    // Returns current camera world-space position. Used for specular in Phase 6.
    glm::vec3 GetPosition() const { return m_Position; }

    // Returns the direction the camera is currently facing (normalized)
    glm::vec3 GetFront() const { return m_Front; }

    // -------------------------------------------------------
    // Input Handlers (called every frame from main loop)
    // -------------------------------------------------------

    // Handle WASD + QE keyboard movement.
    // deltaTime ensures speed is frame-rate independent.
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Handle mouse look. xOffset/yOffset are pixel deltas since last frame.
    // constrainPitch: if true, prevents flipping upside-down (keep true).
    void ProcessMouseMovement(float xOffset, float yOffset,
                              bool constrainPitch = true);

    // Handle scroll wheel for FOV zoom
    void ProcessMouseScroll(float yOffset);

    // Returns current FOV (degrees) — use this for projection matrix
    float GetFOV() const { return m_FOV; }

    // -------------------------------------------------------
    // Settings (tunable at runtime)
    // -------------------------------------------------------
    float MovementSpeed    = 4.0f;   // Units per second — library scale ~12x8 units
    float MouseSensitivity = 0.08f;  // Degrees per pixel
    float SprintMultiplier = 2.5f;   // Applied when Left Shift is held

private:
    // Camera state
    glm::vec3 m_Position;
    glm::vec3 m_Front;    // Direction camera faces (computed from yaw/pitch)
    glm::vec3 m_Up;       // Camera-local up vector (computed)
    glm::vec3 m_Right;    // Camera-local right vector (computed)
    glm::vec3 m_WorldUp;  // Global up (always 0,1,0)

    // Euler angles (degrees)
    float m_Yaw;    // Left/right rotation around Y axis
    float m_Pitch;  // Up/down rotation, clamped to ±89° to prevent gimbal flip

    // FOV for projection matrix (degrees)
    float m_FOV = 60.0f;

    // Recomputes m_Front, m_Right, m_Up from current m_Yaw and m_Pitch.
    // Must be called after any yaw/pitch change.
    void UpdateCameraVectors();
};
```

---

**`src/core/Camera.cpp` — Full Implementation:**

**`UpdateCameraVectors()`:**
```cpp
void Camera::UpdateCameraVectors() {
    // Convert spherical (yaw, pitch) to Cartesian direction vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Recompute right and up using cross products
    // Right = Front × WorldUp (normalized)
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));

    // Up = Right × Front (normalized)
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
```

**`GetViewMatrix()`:**
```cpp
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}
```

**`ProcessKeyboard()`:**
```cpp
void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;

    if (direction == CameraMovement::FORWARD)  m_Position += m_Front * velocity;
    if (direction == CameraMovement::BACKWARD) m_Position -= m_Front * velocity;
    if (direction == CameraMovement::LEFT)     m_Position -= m_Right * velocity;
    if (direction == CameraMovement::RIGHT)    m_Position += m_Right * velocity;
    if (direction == CameraMovement::UP)       m_Position += m_WorldUp * velocity;
    if (direction == CameraMovement::DOWN)     m_Position -= m_WorldUp * velocity;

    // IMPORTANT: Lock Y position to standing height (1.7 units = eye level)
    // Remove this line if you want free-fly mode.
    // TODO: Replace with collision detection in a future phase if desired.
    m_Position.y = 1.7f;
}
```

**`ProcessMouseMovement()`:**
```cpp
void Camera::ProcessMouseMovement(float xOffset, float yOffset,
                                   bool constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    m_Yaw   += xOffset;
    m_Pitch += yOffset;

    if (constrainPitch) {
        // Prevent camera from flipping past vertical
        if (m_Pitch >  89.0f) m_Pitch =  89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }

    UpdateCameraVectors();
}
```

**`ProcessMouseScroll()`:**
```cpp
void Camera::ProcessMouseScroll(float yOffset) {
    m_FOV -= yOffset;                        // Scroll up = zoom in (smaller FOV)
    if (m_FOV < 20.0f) m_FOV = 20.0f;      // Min zoom
    if (m_FOV > 90.0f) m_FOV = 90.0f;      // Max zoom (wide angle)
}
```

---

### 2. Input System — `src/core/InputHandler.h`

Centralize all GLFW input handling. This keeps `main.cpp` clean.

```cpp
#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"

class InputHandler {
public:
    // Call once at startup. Registers GLFW callbacks.
    static void Init(GLFWwindow* window, Camera* camera);

    // Call every frame in main loop. Processes held-down keys for smooth movement.
    // deltaTime: seconds since last frame (for frame-rate-independent movement)
    static void ProcessContinuousInput(GLFWwindow* window, Camera* camera,
                                       float deltaTime);

    // Getters for state that main.cpp might need
    static bool IsMouseCaptured() { return s_MouseCaptured; }

private:
    // GLFW Callbacks (registered via glfwSetCursorPosCallback, etc.)
    static void MouseCallback    (GLFWwindow* w, double x, double y);
    static void ScrollCallback   (GLFWwindow* w, double xOff, double yOff);
    static void KeyCallback      (GLFWwindow* w, int key, int scancode,
                                  int action, int mods);
    static void FramebufferSizeCallback(GLFWwindow* w, int width, int height);

    // State
    static Camera* s_Camera;
    static float   s_LastMouseX;
    static float   s_LastMouseY;
    static bool    s_FirstMouse;     // Prevents jump on first mouse move
    static bool    s_MouseCaptured;  // True when cursor is hidden/locked
    static bool    s_SprintHeld;     // True when Left Shift is down
};
```

**`InputHandler.cpp` implementation notes:**

**`Init()`:**
- Store `camera` pointer in `s_Camera`
- `glfwSetCursorPosCallback(window, MouseCallback)`
- `glfwSetScrollCallback(window, ScrollCallback)`
- `glfwSetKeyCallback(window, KeyCallback)`
- `glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback)`
- `glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED)` — capture mouse on start
- Set `s_MouseCaptured = true`

**`ProcessContinuousInput()`** — called every frame:
```cpp
// Apply sprint multiplier
float speed = s_Camera->MovementSpeed;
if (s_SprintHeld) speed *= s_Camera->SprintMultiplier;
// Temporarily change speed, process, restore:
float saved = s_Camera->MovementSpeed;
s_Camera->MovementSpeed = speed;

if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    s_Camera->ProcessKeyboard(CameraMovement::FORWARD,  deltaTime);
if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    s_Camera->ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    s_Camera->ProcessKeyboard(CameraMovement::LEFT,     deltaTime);
if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    s_Camera->ProcessKeyboard(CameraMovement::RIGHT,    deltaTime);
if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    s_Camera->ProcessKeyboard(CameraMovement::UP,       deltaTime);
if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    s_Camera->ProcessKeyboard(CameraMovement::DOWN,     deltaTime);

s_Camera->MovementSpeed = saved;

// ESC to exit
if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
```

**`MouseCallback()`:**
```cpp
// First mouse move: initialize last position to avoid sudden camera jump
if (s_FirstMouse) {
    s_LastMouseX = (float)x;
    s_LastMouseY = (float)y;
    s_FirstMouse = false;
}

float xOffset =  ((float)x - s_LastMouseX);  // +x = look right
float yOffset =  (s_LastMouseY - (float)y);   // inverted: +y = look up

s_LastMouseX = (float)x;
s_LastMouseY = (float)y;

if (s_MouseCaptured)
    s_Camera->ProcessMouseMovement(xOffset, yOffset);
```

**`ScrollCallback()`:**
```cpp
s_Camera->ProcessMouseScroll((float)yOffset);
```

**`KeyCallback()`** — one-shot key events (not held):
```cpp
// Tab: toggle mouse capture (show/hide cursor)
if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
    s_MouseCaptured = !s_MouseCaptured;
    glfwSetInputMode(window, GLFW_CURSOR,
        s_MouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    s_FirstMouse = true; // Reset to prevent jump when re-capturing
}

// Left Shift: track sprint state
if (key == GLFW_KEY_LEFT_SHIFT) {
    s_SprintHeld = (action != GLFW_RELEASE);
}

// TODO Phase 8: Add L key (toggle lights), N key (night mode) here
```

**`FramebufferSizeCallback()`:**
```cpp
glViewport(0, 0, width, height);
// TODO: Notify main loop to rebuild projection matrix with new aspect ratio
```

---

### 3. Updated `main.cpp`

Replace the static `lookAt` + raw GLFW key checks with the new systems:

```cpp
// Setup:
Camera camera(glm::vec3(0.0f, 1.7f, 5.0f), -90.0f, 0.0f);
InputHandler::Init(window.GetNativeWindow(), &camera);

// Each frame, rebuild projection using camera's current FOV:
glm::mat4 projection = glm::perspective(
    glm::radians(camera.GetFOV()),
    (float)window.GetWidth() / window.GetHeight(),
    0.1f, 100.0f
);

// In render loop:
InputHandler::ProcessContinuousInput(window.GetNativeWindow(), &camera, deltaTime);

shader.SetMat4("u_View",       camera.GetViewMatrix());
shader.SetMat4("u_Projection", projection);
// TODO Phase 6: shader.SetVec3("u_ViewPos", camera.GetPosition());
```

Keep the Phase 3 demo scene (5 cubes + floor) intact — just replace the camera.

---

### 4. Window Title — Live Camera Info

Update the window title every frame with camera position and FPS. Extremely useful for debugging and explaining to your professor what's happening:

```cpp
// Every frame after calculating deltaTime:
float fps = 1.0f / deltaTime;
std::string title = "3D Library | FPS: " + std::to_string((int)fps)
    + " | Pos: ("
    + std::to_string((int)camera.GetPosition().x) + ", "
    + std::to_string((int)camera.GetPosition().y) + ", "
    + std::to_string((int)camera.GetPosition().z) + ")"
    + " | Yaw: " + std::to_string((int)camera.GetYaw());  // add GetYaw() accessor
glfwSetWindowTitle(window.GetNativeWindow(), title.c_str());
```

---

## ✅ ACCEPTANCE CRITERIA

- [ ] W/A/S/D moves the camera smoothly through the scene
- [ ] Mouse look works — moving mouse rotates view without jerking
- [ ] First-mouse protection works — no sudden jump when program starts
- [ ] Left Shift sprints (clearly faster movement)
- [ ] Q/E moves camera up/down (free-fly inspect mode)
- [ ] Scroll wheel zooms in/out (FOV changes visibly)
- [ ] Tab key toggles cursor visibility (mouse captured vs free)
- [ ] ESC closes the window cleanly
- [ ] Camera Y is locked at 1.7 during WASD movement (walking, not flying)
- [ ] Pitch is clamped at ±89° — camera never flips upside down
- [ ] Window title shows live FPS and position
- [ ] No raw `glfwGetKey` calls in `main.cpp` — all input through `InputHandler`

---

## 🚫 STRICT RULES

1. **`s_FirstMouse` protection is mandatory** — failing this causes a horrifying camera snap on first mouse movement
2. **No raw input in `main.cpp`** — everything goes through `InputHandler`
3. **`deltaTime` must be used for movement** — never hardcode a fixed velocity
4. **Pitch clamped to ±89°** — exactly 89, not 90 (90 causes gimbal lock)
5. **Mouse sensitivity must be a configurable constant** — never hardcoded inside `ProcessMouseMovement`

---

## 🔮 FUTURE PHASE HOOKS

- In `Camera.h` below `GetPosition()`:
  `// TODO Phase 6: GetPosition() is used as u_ViewPos uniform for specular highlights`
- In `ProcessKeyboard` Y-lock line:
  `// TODO Phase 10 (optional): Replace with proper floor collision detection`
- In `KeyCallback`:
  `// TODO Phase 8: L key → LightManager toggle, N key → night mode`
- In `InputHandler.h`:
  `// TODO Phase 9: Add F key callback for ceiling fan toggle`

---

## 🗺️ LIBRARY SCENE NAVIGATION NOTES

Your library room (from the reference image) is approximately **12 units wide × 8 units deep × 4.5 units tall**. The camera starting position `(0, 1.7, 5)` places you just inside the front entrance, looking toward the center where the reading tables are. With `MovementSpeed = 4.0`, it takes about 2 seconds to walk the full depth — which feels natural for a room this size.

**Useful positions to visit during testing:**
- `(0, 1.7, 5)` — entrance (start)
- `(0, 1.7, 0)` — center of room between the tables
- `(-5, 1.7, 0)` — standing in front of the left wall bookshelves
- `(0, 4.0, 0)` — aerial view (Q key to fly up)

---

**Build order:** `Camera.h/cpp` (UpdateCameraVectors first) → `InputHandler.h/cpp` → `main.cpp` update → test navigation → verify all acceptance criteria.
