// =============================================================================
// basic.vert — Basic Vertex Shader (OpenGL 3.3 Core)
// =============================================================================
// Transforms vertex positions from model space to clip space using the
// Model-View-Projection (MVP) matrix chain. Passes vertex color to the
// fragment shader for interpolation across the triangle.
//
// Inputs:
//   aPos   (location 0) — vertex position in model space (vec3)
//   aColor (location 1) — vertex color (vec3)
//
// Outputs:
//   vertexColor — interpolated color passed to fragment shader
//
// Uniforms:
//   model      — model matrix (object space → world space)
//   view       — view matrix (world space → camera space)
//   projection — projection matrix (camera space → clip space)
// =============================================================================

#version 330 core

// ---- Vertex Attributes (input from VBO) ----
layout (location = 0) in vec3 aPos;     // Vertex position
layout (location = 1) in vec3 aColor;   // Vertex color

// ---- Output to Fragment Shader ----
out vec3 vertexColor;   // Will be interpolated across the triangle

// ---- Transformation Matrices (set from C++ code) ----
uniform mat4 model;         // Positions and orients the object in the world
uniform mat4 view;          // Positions the camera (inverse of camera transform)
uniform mat4 projection;    // Defines perspective or orthographic projection

void main() {
    // Transform the vertex position through the MVP pipeline:
    // model → world space, view → camera space, projection → clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pass the color through to the fragment shader (will be interpolated)
    vertexColor = aColor;
}
