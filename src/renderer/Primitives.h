// =============================================================================
// Primitives.h — Primitive Geometry Factory (Phase 3)
// =============================================================================
// Provides factory functions that return ready-to-use Mesh objects for
// common 3D shapes. Phase 5 will call these constantly to build furniture,
// walls, and other scene objects from transformed cubes.
//
// Usage:
//   auto cube  = Primitives::CreateCube();
//   auto plane = Primitives::CreatePlane();
//   cube->Draw();
// =============================================================================

#pragma once

#include "renderer/Mesh.h"

#include <memory>               // std::unique_ptr
#include <glm/glm.hpp>          // glm::mat4

// Forward declaration to avoid including Shader.h
class Shader;

namespace Primitives {

    // =========================================================================
    // CreateCube — Returns a unit cube centered at origin
    // =========================================================================
    // Size: 1×1×1 (extends from -0.5 to +0.5 on all axes)
    // Each face has 4 unique vertices with correct outward normals.
    // UV maps (0,0)→(1,1) per face.
    // Use glm::scale(model, glm::vec3(w, h, d)) to resize.
    //
    // Returns: unique_ptr<Mesh> with 24 vertices and 36 indices
    // =========================================================================
    std::unique_ptr<Mesh> CreateCube();

    // =========================================================================
    // CreatePlane — Returns a flat horizontal plane
    // =========================================================================
    // Lies in XZ plane, centered at origin, size 1×1.
    // Normal points +Y. Scale to desired size.
    //
    // Returns: unique_ptr<Mesh> with 4 vertices and 6 indices
    // =========================================================================
    std::unique_ptr<Mesh> CreatePlane();

    // =========================================================================
    // DrawCube — Convenience: sets u_Model uniform and draws a static cube
    // =========================================================================
    // Uses a static cube mesh internally (created on first call).
    // Useful for quick one-off draws without managing mesh lifetime.
    // =========================================================================
    void DrawCube(Shader& shader, const glm::mat4& modelMatrix);

    // TODO Phase 5: Add CreateBookshelf(), CreateChair(), CreateTable(), CreateFan()

} // namespace Primitives
