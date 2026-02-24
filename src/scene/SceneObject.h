// =============================================================================
// SceneObject.h — Lightweight Renderable Object (Phase 5)
// =============================================================================
// Holds everything needed to draw one piece of geometry:
//   - Pre-computed model matrix (TRS, set once at Build time)
//   - Flat color (replaced with Material in Phase 6)
//   - Debug label
// =============================================================================

#pragma once

#include <glm/glm.hpp>
#include <string>

struct SceneObject {
    glm::mat4   Transform;  // Pre-computed model matrix (computed once, not every frame)
    glm::vec3   Color;      // Flat color (used until Phase 6 replaces with Material)
    std::string Label;      // Human-readable name for debugging ("left_wall", "table_1_top")

    // TODO Phase 6: Replace Color with Material (ambient/diffuse/specular/shininess)
};
