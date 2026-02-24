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

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

// Forward declarations
class Shader;
class Mesh;

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

    // Stats
    size_t GetObjectCount() const { return m_Objects.size(); }

    // Update per-frame animations (fan blade rotation)
    void UpdateAnimations(float deltaTime);

private:
    std::unique_ptr<Mesh> m_CubeMesh;       // Single cube mesh shared by all objects
    std::vector<SceneObject> m_Objects;      // All scene objects

    // Camera matrices (set each frame before Render)
    glm::mat4 m_View       = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);

    // Fan animation state
    float m_FanAngle = 0.0f;
    float m_FanSpeed = 120.0f;
    glm::vec3 m_FanCenter = glm::vec3(0.0f);
    std::vector<size_t> m_FanBladeIndices;

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

    // Phase 6: Assign textures and modes after Build
    void AssignTextures();
};
