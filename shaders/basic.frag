// =============================================================================
// BASIC FRAGMENT SHADER — Phase 2
// =============================================================================
// Outputs a solid flat color controlled by uniform u_Color and u_Alpha.
// No lighting. No textures. Just clean, uniform-driven color output.
// This will be REPLACED in Phase 6 with a full Phong lighting shader.
// =============================================================================

#version 330 core

// ---- Inputs from Vertex Shader ----
in vec3 v_FragPos;     // World-space position (unused until Phase 6)
in vec3 v_Normal;      // World-space normal (unused until Phase 6)
in vec2 v_TexCoord;    // UV coordinates (unused until Phase 7)

// ---- Output ----
out vec4 FragColor;    // Final pixel color written to the framebuffer

// ---- Material Uniforms ----
uniform vec3 u_Color;   // Flat object color — set per object from C++ code
uniform float u_Alpha;  // Alpha transparency (1.0 = fully opaque)

void main() {
    // Output solid color with alpha
    FragColor = vec4(u_Color, u_Alpha);
    // TODO Phase 6: Replace flat color output with Phong lighting model
    // TODO Phase 7: Multiply by texture sample: texture(u_DiffuseMap, v_TexCoord).rgb
}
