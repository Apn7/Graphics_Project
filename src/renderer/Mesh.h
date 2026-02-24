// =============================================================================
// Mesh.h — VAO/VBO/EBO Abstraction
// =============================================================================
// Wraps OpenGL vertex array objects, vertex buffers, and element buffers into
// a clean, reusable Mesh class. Handles GPU memory allocation and cleanup.
//
// Usage:
//   std::vector<Vertex> vertices = { ... };
//   std::vector<unsigned int> indices = { ... };
//   Mesh cube(vertices, indices);
//   cube.Draw();           // Renders the mesh
//   cube.Delete();         // Frees GPU memory
// =============================================================================

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>      // glm::vec3, glm::vec2

// =============================================================================
// Vertex — Represents a single vertex with all its attributes
// =============================================================================
// Currently stores Position and Color. Additional attributes will be added
// in future phases.
// =============================================================================
struct Vertex {
    glm::vec3 Position;     // Vertex position in 3D space (x, y, z)
    glm::vec3 Color;        // Vertex color (r, g, b)
    // TODO Phase 6: glm::vec3 Normal;    — surface normal for lighting
    // TODO Phase 7: glm::vec2 TexCoord;  — texture coordinates (u, v)
};

class Mesh {
public:
    // =========================================================================
    // Constructor — Creates a mesh from vertex and index data
    // =========================================================================
    // Parameters:
    //   vertices — array of Vertex structs defining the mesh geometry
    //   indices  — array of indices for indexed drawing (EBO)
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
    // Binds the VAO and calls glDrawElements to render all indexed triangles.
    // =========================================================================
    void Draw() const;

    // =========================================================================
    // Delete — Explicitly frees GPU memory (VAO, VBO, EBO)
    // =========================================================================
    // Called automatically by the destructor, but can be called manually
    // for early resource release.
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

#endif // MESH_H
