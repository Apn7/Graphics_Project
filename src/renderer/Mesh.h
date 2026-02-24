// =============================================================================
// Mesh.h — VAO/VBO/EBO Abstraction (Phase 2)
// =============================================================================
// Wraps OpenGL vertex array objects, vertex buffers, and element buffers into
// a clean, reusable Mesh class. Handles GPU memory allocation and cleanup.
//
// Phase 2 Update: Vertex struct now uses Position + Normal + TexCoord
// instead of Position + Color. Color is now set via shader uniform u_Color.
//
// Usage:
//   std::vector<Vertex> vertices = { ... };
//   std::vector<unsigned int> indices = { ... };
//   Mesh cube(vertices, indices);
//   cube.Draw();           // Renders the mesh
//   cube.Delete();         // Frees GPU memory
// =============================================================================

#pragma once

#include <vector>
#include <glm/glm.hpp>      // glm::vec3, glm::vec2

// =============================================================================
// Vertex — Represents a single vertex with all its attributes
// =============================================================================
// Phase 2: Position + Normal + TexCoord
// The vertex layout matches the shader attribute layout:
//   location 0 = aPos      (vec3)
//   location 1 = aNormal   (vec3)
//   location 2 = aTexCoord (vec2)
// =============================================================================
struct Vertex {
    glm::vec3 Position;     // Vertex position in 3D space (x, y, z)
    glm::vec3 Normal;       // Surface normal for lighting (Phase 6)
    glm::vec2 TexCoord;     // Texture coordinates (Phase 7)
};

class Mesh {
public:
    // =========================================================================
    // Constructor — Creates a mesh from vertex and index data
    // =========================================================================
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    // =========================================================================
    // Destructor — Automatically cleans up GPU resources
    // =========================================================================
    ~Mesh();

    // --- Prevent copying (GPU resources should not be duplicated) ---
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // --- Allow move semantics for flexibility ---
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // =========================================================================
    // Draw — Renders the mesh using the currently bound shader
    // =========================================================================
    void Draw() const;

    // =========================================================================
    // Delete — Explicitly frees GPU memory (VAO, VBO, EBO)
    // =========================================================================
    void Delete();

private:
    unsigned int m_VAO;         // Vertex Array Object — stores attribute layout
    unsigned int m_VBO;         // Vertex Buffer Object — stores vertex data
    unsigned int m_EBO;         // Element Buffer Object — stores index data
    unsigned int m_IndexCount;  // Number of indices (= number of elements to draw)

    // =========================================================================
    // SetupMesh — Configures VAO, VBO, EBO and vertex attribute pointers
    // =========================================================================
    void SetupMesh(const std::vector<Vertex>& vertices,
                   const std::vector<unsigned int>& indices);
};
