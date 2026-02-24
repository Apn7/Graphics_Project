// =============================================================================
// TextureMode.h — Texture Rendering Modes (Phase 6)
// =============================================================================
// Defines how each object is rendered with respect to texturing.
// Assignment requirement: demonstrate all 3 modes + untextured fallback.
// =============================================================================

#pragma once

// Per-object texture mode
enum class TextureMode {
    FLAT_COLOR      = 0,  // No texture — original Phase 5 flat color (fallback)
    SIMPLE_TEXTURE  = 1,  // Texture only — no surface color modification
    VERTEX_BLEND    = 2,  // Texture × color computed at vertex stage
    FRAGMENT_BLEND  = 3,  // Texture × color computed at fragment stage
};

// Global override — when set, ALL objects render in this mode regardless of
// their assigned mode. When NONE, each object uses its own assigned mode.
enum class GlobalTextureOverride {
    NONE            = -1,  // Per-object mode (default)
    ALL_FLAT        = 0,   // Override: everything renders flat color
    ALL_SIMPLE      = 1,   // Override: everything uses simple texture
    ALL_VERTEX      = 2,   // Override: everything uses vertex blend
    ALL_FRAGMENT    = 3,   // Override: everything uses fragment blend
};
