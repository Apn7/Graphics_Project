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

// =============================================================================
// CreateSphere — UV parametric sphere, radius=1, centered at origin (Phase 8)
// =============================================================================
// Generation strategy:
//   For each (stack, slice) pair, compute a vertex using spherical coordinates:
//     x = sin(phi) * cos(theta)
//     y = cos(phi)                 (phi = latitude, 0 at top, π at bottom)
//     z = sin(phi) * sin(theta)   (theta = longitude)
//   Normal = position (for a unit sphere, the outward normal equals the position)
//   UV = (theta / 2π, phi / π)  → equirectangular — works perfectly with world maps
// =============================================================================
std::unique_ptr<Mesh> CreateSphere(int stacks, int slices) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265358979323846f;

    for (int i = 0; i <= stacks; i++) {
        float phi = PI * static_cast<float>(i) / static_cast<float>(stacks); // 0 → π
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        float v = static_cast<float>(i) / static_cast<float>(stacks); // UV v: 0 → 1

        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(slices); // 0 → 2π
            float u = static_cast<float>(j) / static_cast<float>(slices); // UV u: 0 → 1

            glm::vec3 pos = {
                sinPhi * std::cos(theta),
                cosPhi,
                sinPhi * std::sin(theta)
            };
            glm::vec3 normal = pos; // Unit sphere: normal = position
            glm::vec2 uv     = { u, v };

            vertices.push_back({ pos, normal, uv });
        }
    }

    // Build indices — two triangles per quad cell
    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            unsigned int tl = i       * (slices + 1) + j;       // top-left
            unsigned int tr = i       * (slices + 1) + j + 1;   // top-right
            unsigned int bl = (i + 1) * (slices + 1) + j;       // bottom-left
            unsigned int br = (i + 1) * (slices + 1) + j + 1;   // bottom-right

            // Triangle 1
            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);
            // Triangle 2
            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }

    LOG_INFO("Primitives: Created sphere (" + std::to_string(vertices.size()) +
             " vertices, " + std::to_string(indices.size() / 3) + " triangles)");
    return std::make_unique<Mesh>(vertices, indices);
}

// =============================================================================
// CreateCone — Ruled surface: lateral triangles + base cap (Phase 8)
// =============================================================================
// A cone is the simplest ruled surface:
//   - Every lateral triangle has one edge on the base circle and one vertex at apex
//   - The "ruling" lines are the edges from circle to apex
// Height = 1 (apex at Y=1, base at Y=0, radius=1 at base).
// Scale in the scene to fit as lampshade: scale(baseRadius, height, baseRadius).
// Normals on lateral face are perpendicular to the slant surface.
// =============================================================================
std::unique_ptr<Mesh> CreateCone(int slices) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265358979323846f;
    const float apexY = 1.0f;
    const float baseY = 0.0f;
    const float radius = 1.0f;

    // Slant angle for lateral normals (the cone makes a 45° half-angle when r=h=1)
    // Normal of each lateral face: perpendicular to the slant edge
    // n = normalize( cosSlant * radial_dir + sinSlant * up )
    // For r=1, h=1, slantAngle from horizontal = atan(1/1) = 45°
    float cosSlant = std::cos(PI / 4.0f); // = sqrt(2)/2
    float sinSlant = std::sin(PI / 4.0f);

    // ---- Lateral surface ----
    // For each slice: apex vertex + two base vertices = one triangle
    unsigned int idx = 0;
    for (int j = 0; j < slices; j++) {
        float theta0 = 2.0f * PI * static_cast<float>(j)     / static_cast<float>(slices);
        float theta1 = 2.0f * PI * static_cast<float>(j + 1) / static_cast<float>(slices);

        float u0 = static_cast<float>(j)     / static_cast<float>(slices);
        float u1 = static_cast<float>(j + 1) / static_cast<float>(slices);

        glm::vec3 base0 = { radius * std::cos(theta0), baseY, radius * std::sin(theta0) };
        glm::vec3 base1 = { radius * std::cos(theta1), baseY, radius * std::sin(theta1) };
        glm::vec3 apex  = { 0.0f, apexY, 0.0f };

        // Lateral face normal: average of the two edge normals, points outward+up
        // Outward direction at base0 and base1 (radial component)
        glm::vec3 rDir0 = glm::normalize(glm::vec3(std::cos(theta0), 0.0f, std::sin(theta0)));
        glm::vec3 rDir1 = glm::normalize(glm::vec3(std::cos(theta1), 0.0f, std::sin(theta1)));
        glm::vec3 rDirM = glm::normalize(glm::vec3(
            std::cos((theta0 + theta1) * 0.5f), 0.0f, std::sin((theta0 + theta1) * 0.5f)));

        glm::vec3 n0 = glm::normalize(glm::vec3(rDir0.x * cosSlant, sinSlant, rDir0.z * cosSlant));
        glm::vec3 n1 = glm::normalize(glm::vec3(rDir1.x * cosSlant, sinSlant, rDir1.z * cosSlant));
        glm::vec3 nA = glm::normalize(glm::vec3(rDirM.x * cosSlant, sinSlant, rDirM.z * cosSlant));

        // base0 → base1 → apex (counter-clockwise from outside)
        vertices.push_back({ base0, n0, { u0, 0.0f } });
        vertices.push_back({ base1, n1, { u1, 0.0f } });
        vertices.push_back({ apex,  nA, { (u0 + u1) * 0.5f, 1.0f } });

        indices.push_back(idx);
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
        idx += 3;
    }

    // ---- Base cap (disk) — normal points DOWN (-Y) ----
    glm::vec3 baseCenterPos = { 0.0f, baseY, 0.0f };
    glm::vec3 baseNormal    = { 0.0f, -1.0f, 0.0f };
    unsigned int centerIdx  = idx;
    vertices.push_back({ baseCenterPos, baseNormal, { 0.5f, 0.5f } });
    idx++;

    for (int j = 0; j <= slices; j++) {
        float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(slices);
        glm::vec3 p = { radius * std::cos(theta), baseY, radius * std::sin(theta) };
        glm::vec2 uv = { 0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta) };
        vertices.push_back({ p, baseNormal, uv });

        if (j < slices) {
            indices.push_back(centerIdx);
            indices.push_back(idx + 1);
            indices.push_back(idx);
        }
        idx++;
    }

    LOG_INFO("Primitives: Created cone (" + std::to_string(vertices.size()) +
             " vertices, " + std::to_string(indices.size() / 3) + " triangles)");
    return std::make_unique<Mesh>(vertices, indices);
}

// =============================================================================
// CreateBezierVase — Bezier profile revolved 360° (Phase 8)
// =============================================================================
// Uses a CUBIC Bezier curve to define the vase silhouette (profile in 2D: radius vs height).
// The profile is then revolved (surface of revolution) around the Y axis to produce a 3D mesh.
//
// Bezier formula (cubic, parameter t ∈ [0,1]):
//   B(t) = (1-t)³ P0 + 3(1-t)²t P1 + 3(1-t)t² P2 + t³ P3
//
// Profile control points (radius, height):
//   P0 = base of vase    (narrow base foot)
//   P1 = lower bulge     (wide belly)
//   P2 = waist           (narrow neck transition)
//   P3 = rim             (slightly flared lip)
//
// Normal at each vertex is computed from the tangent of the profile curve
// crossed with the tangent of the circle at that radial angle.
// =============================================================================
std::unique_ptr<Mesh> CreateBezierVase(int profileSteps, int radialSlices) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265358979323846f;

    // ---- Bezier control points: (radius, height) ----
    // Each is a 2D point. Designed so the vase is ~1 unit tall and ~0.5 wide at belly.
    // Tweak these to change shape. The vase will be scaled in the scene.
    const glm::vec2 P0 = { 0.10f, 0.00f };   // Base foot — narrow
    const glm::vec2 P1 = { 0.50f, 0.20f };   // Lower belly — wide bulge
    const glm::vec2 P2 = { 0.45f, 0.70f };   // Upper belly
    const glm::vec2 P3 = { 0.20f, 1.00f };   // Rim — slightly narrower
    // Extra segments: we'll use a multi-segment Bezier for the full vase shape
    // Segment 2: rim → neck → opening
    const glm::vec2 P4 = { 0.15f, 1.10f };   // Neck
    const glm::vec2 P5 = { 0.22f, 1.18f };   // Lip flare

    // We'll sample the profile by evaluating two cubic Bezier segments:
    // Segment A: P0→P1→P2→P3  (bottom half: foot to rim)
    // Segment B: P3→P4→P5→P5  (top rim: neck + lip)

    auto CubicBezier = [](float t, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) -> glm::vec2 {
        float t2 = t * t, t3 = t2 * t;
        float mt = 1.0f - t, mt2 = mt * mt, mt3 = mt2 * mt;
        return mt3 * a + 3.0f * mt2 * t * b + 3.0f * mt * t2 * c + t3 * d;
    };

    auto CubicBezierTangent = [](float t, glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) -> glm::vec2 {
        float mt = 1.0f - t;
        return 3.0f * (mt * mt * (b - a) + 2.0f * mt * t * (c - b) + t * t * (d - c));
    };

    // Build full profile samples: half in each segment
    int halfSteps = profileSteps / 2;
    std::vector<glm::vec2> profile; // (radius, height) samples
    std::vector<glm::vec2> tangents;

    for (int i = 0; i <= halfSteps; i++) {
        float t = static_cast<float>(i) / static_cast<float>(halfSteps);
        profile.push_back(CubicBezier(t, P0, P1, P2, P3));
        tangents.push_back(CubicBezierTangent(t, P0, P1, P2, P3));
    }
    for (int i = 1; i <= profileSteps - halfSteps; i++) {
        float t = static_cast<float>(i) / static_cast<float>(profileSteps - halfSteps);
        profile.push_back(CubicBezier(t, P3, P4, P5, P5));
        tangents.push_back(CubicBezierTangent(t, P3, P4, P5, P5));
    }

    int rows = static_cast<int>(profile.size());

    // ---- Generate vertices by revolving the profile ----
    for (int i = 0; i < rows; i++) {
        float r = std::max(profile[i].x, 0.001f); // radius (keep positive)
        float y = profile[i].y;                    // height

        float v = static_cast<float>(i) / static_cast<float>(rows - 1);

        // Tangent of the profile curve in (r, y) space
        glm::vec2 tang2D = tangents[i];
        // The outward normal in 2D: rotate tangent 90° → (-tang2D.y, tang2D.x)
        // tang2D.x = dr/dt (radial component), tang2D.y = dy/dt (height component)
        // Normal in (r, y): perpendicular to tangent = (-tang2D.y, tang2D.x) normalized
        glm::vec2 norm2D = glm::normalize(glm::vec2(-tang2D.y, tang2D.x));
        // norm2D.x = outward radial factor, norm2D.y = upward factor

        for (int j = 0; j <= radialSlices; j++) {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(radialSlices);
            float cosT  = std::cos(theta);
            float sinT  = std::sin(theta);
            float u     = static_cast<float>(j) / static_cast<float>(radialSlices);

            glm::vec3 pos    = { r * cosT, y, r * sinT };
            glm::vec3 normal = glm::normalize(glm::vec3(norm2D.x * cosT, norm2D.y, norm2D.x * sinT));
            glm::vec2 uv     = { u, v };

            vertices.push_back({ pos, normal, uv });
        }
    }

    // ---- Build indices ----
    int cols = radialSlices + 1;
    for (int i = 0; i < rows - 1; i++) {
        for (int j = 0; j < radialSlices; j++) {
            unsigned int tl = i       * cols + j;
            unsigned int tr = i       * cols + j + 1;
            unsigned int bl = (i + 1) * cols + j;
            unsigned int br = (i + 1) * cols + j + 1;

            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }

    LOG_INFO("Primitives: Created Bezier vase (" + std::to_string(vertices.size()) +
             " vertices, " + std::to_string(indices.size() / 3) + " triangles)");
    return std::make_unique<Mesh>(vertices, indices);
}

} // namespace Primitives

