// =============================================================================
// SceneObject.h — Lightweight Renderable Object (Phase 5 + Phase 6 Textures)
// =============================================================================
// Holds everything needed to draw one piece of geometry:
//   - Pre-computed model matrix (TRS, set once at Build time)
//   - Flat color or surface color for blending
//   - Texture ID and mode (Phase 6)
//   - Debug label
// =============================================================================

#pragma once

#include <glm/glm.hpp>
#include <string>
#include "TextureMode.h"

struct SceneObject {
    glm::mat4   Transform;      // Pre-computed model matrix
    glm::vec3   Color;          // Surface color (flat color or blend tint)
    std::string Label;          // Human-readable name for debugging

    // Texture fields (Phase 6):
    unsigned int TextureID   = 0;                       // OpenGL texture ID (0 = no texture)
    TextureMode  Mode        = TextureMode::FLAT_COLOR;  // How this object is rendered
    float        UVTileX     = 1.0f;                     // Texture repeat count X
    float        UVTileY     = 1.0f;                     // Texture repeat count Y
    float        BlendFactor = 0.5f;                     // 0.0 = full texture, 1.0 = full color

    // TODO Phase 7: Add Material struct for ambient/diffuse/specular/shininess
};
