// =============================================================================
// Primitives.h — Primitive Geometry Factory (Phase 3 + Phase 8 Curved Objects)
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

    // =========================================================================
    // CreateSphere — UV sphere (parametric surface of revolution)
    // =========================================================================
    // Generates a unit sphere (radius = 1) centered at origin.
    // stacks = latitude divisions, slices = longitude divisions.
    // UV: u along longitude (0→1), v along latitude (0→1) — maps to equirectangular textures.
    // Phase 8: Used for the textured globe on the reading table.
    //
    // Returns: unique_ptr<Mesh>
    // =========================================================================
    std::unique_ptr<Mesh> CreateSphere(int stacks = 32, int slices = 32);

    // =========================================================================
    // CreateCone — Ruled surface (circle base → tip point)
    // =========================================================================
    // Generates a closed cone: lateral surface + circular base cap.
    // Height along +Y axis, apex at (0, height, 0), base at Y=0 radius=1.
    // Phase 8: Used as the pendant lampshade geometry.
    //
    // Returns: unique_ptr<Mesh>
    // =========================================================================
    std::unique_ptr<Mesh> CreateCone(int slices = 32);

    // =========================================================================
    // CreateBezierVase — Surface of revolution from a Bezier profile curve
    // =========================================================================
    // Builds a vase by:
    //   1. Evaluating a cubic Bezier curve in 2D (radius vs height)
    //   2. Revolving it 360° around the Y-axis (surface of revolution)
    // Control points define the silhouette — tweak them to change shape.
    // Phase 8: Decorative vase placed near the library door.
    // The vase is designed tall enough to hold a fractal plant later.
    //
    // Returns: unique_ptr<Mesh>
    // =========================================================================
    std::unique_ptr<Mesh> CreateBezierVase(int profileSteps = 40, int radialSlices = 48);

} // namespace Primitives
