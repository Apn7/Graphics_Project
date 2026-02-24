// =============================================================================
// Scene.h — Library Scene Manager (Phase 5)
// =============================================================================
// Manages the complete library scene — walls, shelves, books, tables, chairs,
// fans, windows, door. All objects are transformed cubes.
// =============================================================================

#pragma once

#include "scene/SceneObject.h"

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

    // Draw all objects. Shader must already be Use()'d before calling.
    void Render(Shader& shader);

    // Draw only objects whose Label starts with groupPrefix
    void RenderGroup(Shader& shader, const std::string& groupPrefix);

    // Stats
    size_t GetObjectCount() const { return m_Objects.size(); }

    // TODO Phase 9: Add UpdateAnimations(float deltaTime) method for fan rotation

private:
    std::unique_ptr<Mesh> m_CubeMesh;       // Single cube mesh shared by all objects
    std::vector<SceneObject> m_Objects;      // All scene objects

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
    void BuildIslandShelf();
    void BuildTables();
    void BuildChairs();
    void BuildCeiling();
    void BuildBooks();
};
