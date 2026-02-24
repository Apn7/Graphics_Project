// =============================================================================
// basic.frag — Basic Fragment Shader (OpenGL 3.3 Core)
// =============================================================================
// Receives the interpolated vertex color from the vertex shader and outputs
// it as the final pixel color. This is the simplest possible fragment shader.
//
// Inputs:
//   vertexColor — interpolated color from vertex shader (vec3)
//
// Outputs:
//   FragColor — final pixel color written to the framebuffer (vec4)
//
// TODO Phase 6: Add lighting calculations (ambient, diffuse, specular)
// TODO Phase 7: Add texture sampling and blending
// =============================================================================

#version 330 core

// ---- Input from Vertex Shader ----
in vec3 vertexColor;    // Interpolated color (different for each pixel)

// ---- Output ----
out vec4 FragColor;     // Final color for this pixel (RGBA)

void main() {
    // Output the interpolated color with full opacity (alpha = 1.0)
    FragColor = vec4(vertexColor, 1.0);
}
