// =============================================================================
// BASIC VERTEX SHADER — Phase 2
// =============================================================================
// Transforms vertices from object space to clip space via the MVP pipeline.
// Accepts position, normal, and UV attributes. Passes world-space position,
// normal, and UVs to the fragment shader for future use.
//
// Normals and UVs are unused visually until Phases 6 & 7, but the layout
// is established now so the VAO stays compatible across all phases.
// =============================================================================

#version 330 core

// ---- Vertex Attributes (must match VAO attribute pointers in Mesh) ----
layout (location = 0) in vec3 aPos;       // Object-space vertex position
layout (location = 1) in vec3 aNormal;    // Surface normal (for Phase 6 lighting)
layout (location = 2) in vec2 aTexCoord;  // UV texture coordinate (for Phase 7)

// ---- Outputs to Fragment Shader ----
out vec3 v_FragPos;     // World-space position (lighting uses this in Phase 6)
out vec3 v_Normal;      // World-space normal (lighting uses this in Phase 6)
out vec2 v_TexCoord;    // UV passed through (texture sampling in Phase 7)

// ---- Transform Uniforms ----
uniform mat4 u_Model;       // Object space → World space
uniform mat4 u_View;        // World space → Camera space
uniform mat4 u_Projection;  // Camera space → Clip space (perspective)

// TODO Phase 6: Uncomment u_NormalMatrix uniform and use it for correct normals
// under non-uniform scaling: uniform mat3 u_NormalMatrix;

void main() {
    // Compute world-space position (used for lighting in Phase 6)
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    v_FragPos = vec3(worldPos);

    // Transform the normal to world space
    // transpose(inverse(mat3(u_Model))) corrects for non-uniform scaling
    // In Phase 6, this will be replaced with a CPU-computed u_NormalMatrix
    v_Normal = mat3(transpose(inverse(u_Model))) * aNormal;

    // Pass UVs through unchanged
    v_TexCoord = aTexCoord;

    // Final clip-space position for rasterization
    gl_Position = u_Projection * u_View * worldPos;
}
