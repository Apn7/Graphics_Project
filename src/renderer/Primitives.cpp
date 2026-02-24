// =============================================================================
// Primitives.cpp — Primitive Geometry Implementation (Phase 3)
// =============================================================================
// Implements factory functions for creating cube and plane meshes with
// correct vertex positions, normals, and UV coordinates.
// =============================================================================

#include "renderer/Primitives.h"
#include "core/Shader.h"
#include "utils/Logger.h"

#include <vector>

namespace Primitives {

// =============================================================================
// CreateCube — Unit cube centered at origin, 24 vertices, 36 indices
// =============================================================================
// Each face has 4 unique vertices because faces do NOT share vertices —
// each face needs its own unique normal vector for correct lighting in Phase 6.
//
// Index pattern per face (B = face_number * 4):
//   Triangle 1: B+0, B+1, B+2
//   Triangle 2: B+2, B+3, B+0
// =============================================================================
std::unique_ptr<Mesh> CreateCube() {
    std::vector<Vertex> vertices = {
        // --- Front face (+Z) --- Normal: (0, 0, +1)
        {{ -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},  // 0
        {{  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},  // 1
        {{  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},  // 2
        {{ -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }},  // 3

        // --- Back face (-Z) --- Normal: (0, 0, -1)
        {{  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }}, // 4
        {{ -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }}, // 5
        {{ -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }}, // 6
        {{  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }}, // 7

        // --- Left face (-X) --- Normal: (-1, 0, 0)
        {{ -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }}, // 8
        {{ -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }}, // 9
        {{ -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }}, // 10
        {{ -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }}, // 11

        // --- Right face (+X) --- Normal: (+1, 0, 0)
        {{  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},  // 12
        {{  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }},  // 13
        {{  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }},  // 14
        {{  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},  // 15

        // --- Top face (+Y) --- Normal: (0, +1, 0)
        {{ -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},  // 16
        {{  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},  // 17
        {{  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},  // 18
        {{ -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},  // 19

        // --- Bottom face (-Y) --- Normal: (0, -1, 0)
        {{ -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }}, // 20
        {{  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }}, // 21
        {{  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}, // 22
        {{ -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }}, // 23
    };

    // 36 indices — 2 triangles per face × 6 faces
    std::vector<unsigned int> indices = {
        // Front face
         0,  1,  2,    2,  3,  0,
        // Back face
         4,  5,  6,    6,  7,  4,
        // Left face
         8,  9, 10,   10, 11,  8,
        // Right face
        12, 13, 14,   14, 15, 12,
        // Top face
        16, 17, 18,   18, 19, 16,
        // Bottom face
        20, 21, 22,   22, 23, 20,
    };

    LOG_INFO("Primitives: Created unit cube (24 vertices, 36 indices)");
    return std::make_unique<Mesh>(vertices, indices);
}

// =============================================================================
// CreatePlane — Flat quad in XZ plane, size 1×1, normal +Y
// =============================================================================
// 4 vertices, 6 indices (2 triangles).
// Centered at origin, lies at y=0.
// Scale with Transform::TRS to desired floor/ceiling size.
// =============================================================================
std::unique_ptr<Mesh> CreatePlane() {
    std::vector<Vertex> vertices = {
        // Position                          Normal              UV
        {{ -0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},  // 0: back-left
        {{  0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},  // 1: back-right
        {{  0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},  // 2: front-right
        {{ -0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }},  // 3: front-left
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,    // Triangle 1
        2, 3, 0,    // Triangle 2
    };

    LOG_INFO("Primitives: Created plane (4 vertices, 6 indices)");
    return std::make_unique<Mesh>(vertices, indices);
}

// =============================================================================
// DrawCube — Convenience function: sets u_Model and draws a static cube
// =============================================================================
// Creates a static cube mesh on first call (lazy initialization).
// Sets the shader's u_Model uniform and draws the cube.
// =============================================================================
void DrawCube(Shader& shader, const glm::mat4& modelMatrix) {
    // Static cube — created once, persists for the lifetime of the program
    static std::unique_ptr<Mesh> s_Cube = CreateCube();

    shader.SetMat4("u_Model", modelMatrix);
    s_Cube->Draw();
}

} // namespace Primitives
