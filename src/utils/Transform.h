// =============================================================================
// Transform.h — Model Matrix Builder Utility (Phase 3)
// =============================================================================
// Header-only helpers for building model matrices in a single line.
// Phase 5 will call TRS() hundreds of times to place furniture.
//
// Usage:
//   glm::mat4 m = Transform::TRS({0, 1, 0}, {0, 45, 0}, {2, 0.1f, 2});
//   glm::mat4 m = Transform::TS({5, 0, 3}, 2.0f);
//   glm::mat4 m = Transform::T({1, 2, 3});
// =============================================================================

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>     // translate, rotate, scale

namespace Transform {

    // =========================================================================
    // TRS — Build a complete model matrix from Translate, Rotate, Scale
    // =========================================================================
    // Parameters:
    //   position        — world-space position (x, y, z)
    //   rotationDegrees — rotation angles in degrees around X, Y, Z axes
    //   scale           — scale factors along X, Y, Z axes
    // Returns:
    //   A mat4 model matrix applying: Translate → RotateX → RotateY → RotateZ → Scale
    // =========================================================================
    inline glm::mat4 TRS(
        const glm::vec3& position,
        const glm::vec3& rotationDegrees = glm::vec3(0.0f),
        const glm::vec3& scale           = glm::vec3(1.0f))
    {
        glm::mat4 m = glm::mat4(1.0f);                                     // Identity
        m = glm::translate(m, position);                                     // T
        m = glm::rotate(m, glm::radians(rotationDegrees.x), glm::vec3(1, 0, 0)); // Rx
        m = glm::rotate(m, glm::radians(rotationDegrees.y), glm::vec3(0, 1, 0)); // Ry
        m = glm::rotate(m, glm::radians(rotationDegrees.z), glm::vec3(0, 0, 1)); // Rz
        m = glm::scale(m, scale);                                            // S
        return m;
    }

    // =========================================================================
    // TS — Shorthand: position + uniform scale (no rotation)
    // =========================================================================
    inline glm::mat4 TS(const glm::vec3& position, float uniformScale) {
        return TRS(position, glm::vec3(0.0f), glm::vec3(uniformScale));
    }

    // =========================================================================
    // T — Shorthand: position only (identity rotation and scale)
    // =========================================================================
    inline glm::mat4 T(const glm::vec3& position) {
        return glm::translate(glm::mat4(1.0f), position);
    }

    // TODO Phase 9: Add parent-child transform composition for fan blade animation

} // namespace Transform
