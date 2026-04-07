// =============================================================================
// Scene.h — Library Scene Manager (Phase 5 + Phase 6 Textures)
// =============================================================================
// Manages the complete library scene — walls, shelves, books, tables, chairs,
// fans, windows, door. All objects are transformed cubes.
// Phase 6: Added texture assignment, multi-shader render, camera matrix storage.
// =============================================================================

#pragma once

#include "scene/SceneObject.h"
#include "scene/TextureMode.h"
#include "scene/LightState.h"

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

// Forward declarations
class Shader;
class Mesh;

// =============================================================================
// CurvedObject — A scene object with its OWN mesh (sphere, cone, vase, etc.)
// =============================================================================
// Unlike SceneObject (which all share m_CubeMesh), curved objects each carry
// a unique procedurally generated mesh. Stored separately and rendered after
// the main SceneObject loop.
// =============================================================================
struct CurvedObject {
    std::unique_ptr<Mesh>  Mesh;           // Owns its mesh
    glm::mat4              Transform = glm::mat4(1.0f);      // Pre-computed model matrix
    glm::vec3              Color = glm::vec3(1.0f);          // Flat/surface color
    std::string            Label;          // Debug name
    unsigned int           TextureID = 0;  // 0 = flat color
    TextureMode            Mode = TextureMode::FLAT_COLOR;
    float                  BlendFactor = 0.0f;
};

class Scene {
public:
    Scene();
    ~Scene();  // Defined in Scene.cpp (unique_ptr<Mesh> needs full type)

    // Build the entire library. Called once at startup.
    void Build();

    // --- Phase 6: Multi-shader render ---
    // Each object picks the correct shader based on its TextureMode.
    void Render(Shader& flatShader,
                Shader& simpleShader,
                Shader& vertexBlendShader,
                Shader& fragmentBlendShader,
                GlobalTextureOverride override);

    // Legacy single-shader render (kept for RenderGroup / debug views)
    void Render(Shader& shader);

    // Draw only objects whose Label starts with groupPrefix
    void RenderGroup(Shader& shader, const std::string& groupPrefix);

    // Store camera matrices so Render() can set per-shader uniforms
    void SetCameraMatrices(const glm::mat4& view, const glm::mat4& projection);

    // Phase 7: Upload all LightState uniforms to any given shader (call before Render)
    static void SetLighting(Shader& shader, const LightState& lights, const glm::vec3& viewPos);

    // Stats
    size_t GetObjectCount() const { return m_Objects.size(); }

    // Update per-frame animations (fan, door, windows)
    // fanDeltaTime is 0.0 when fan is paused; globalDeltaTime always ticks
    void UpdateAnimations(float fanDeltaTime, float globalDeltaTime);

    // Phase 8: Interactives Toggles
    void ToggleDoor() { m_DoorOpen = !m_DoorOpen; }
    void ToggleWindows() { m_WindowsOpen = !m_WindowsOpen; }

private:
    std::unique_ptr<Mesh> m_CubeMesh;           // Single cube mesh shared by all cube objects
    std::vector<SceneObject>   m_Objects;        // All cube-based scene objects
    std::vector<CurvedObject>  m_CurvedObjects;  // Phase 8: sphere, cone, vase (own meshes)

    // Camera matrices (set each frame before Render)
    glm::mat4 m_View       = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);

    // Globe rotation state
    float m_GlobeAngle = 0.0f;
    float m_GlobeSpeed = 12.0f;  // degrees per second (slow, realistic)

    // Fan animation state
    float m_FanAngle = 0.0f;
    float m_FanSpeed = 120.0f;
    std::vector<glm::vec3> m_FanCenters;      // One per fan (9 total in 3x3 FLFLF grid)
    std::vector<size_t>    m_FanBladeIndices; // 4 blades per fan; fan k uses [k*4 .. k*4+3]

    // Door animation state
    bool m_DoorOpen = false;
    float m_DoorAngle = 0.0f;           // 0.0 to 110.0 degrees
    std::vector<size_t> m_DoorIndices;  // indices in m_Objects (door, handle)

    // Window animation state
    bool m_WindowsOpen = false;
    float m_WindowAngle = 0.0f;          // 0 → 85 degrees (each shutter swings inward)
    std::vector<size_t> m_WindowIndices; // [0..3]=panels, [4..7]=inner knobs, [8..11]=outer knobs
                                         // panels: LW-left, LW-right, RW-left, RW-right

    // Helper: adds a SceneObject (no rotation)
    void Add(const std::string& label,
             const glm::vec3& position,
             const glm::vec3& scale,
             const glm::vec3& color);

    // Helper: adds a SceneObject (with rotation)
    void Add(const std::string& label,
             const glm::vec3& position,
             const glm::vec3& rotDeg,
             const glm::vec3& scale,
             const glm::vec3& color);

    // --- Group builders (called by Build()) ---
    void BuildRoom();
    void BuildWindows();
    void BuildDoor();
    void BuildWallShelves();
    void BuildTables();
    void BuildChairs();
    void BuildCeiling();
    void BuildBooks();
    void BuildPendantLamps();    // Phase 7: 3 hanging lamps above tables
    void BuildCurvedObjects();   // Phase 8: sphere (globe), cone (lampshade), vase (by door)
    void BuildFractalTree();     // Phase 9: recursive fractal plant growing from the vase

    // Phase 6: Assign textures and modes after Build
    void AssignTextures();
};
