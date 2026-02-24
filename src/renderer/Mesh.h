// =============================================================================
// Mesh.h — VAO/VBO/EBO Abstraction (Phase 3)
// =============================================================================
// Wraps OpenGL vertex array objects, vertex buffers, and element buffers.
// Handles GPU memory allocation, attribute layout, and cleanup.
//
// Phase 3 Update: Added GetVertexCount/GetIndexCount accessors,
// renamed Delete→Release for clearer ownership semantics.
// =============================================================================

#pragma once

#include <vector>
#include <glm/glm.hpp>      // glm::vec3, glm::vec2

// =============================================================================
// Vertex — Single vertex format used throughout the entire project
// =============================================================================
// Matches the shader attribute layout:
//   location 0 = aPos      (vec3)
//   location 1 = aNormal   (vec3)
//   location 2 = aTexCoord (vec2)
// =============================================================================
struct Vertex {
    glm::vec3 Position;     // Object-space XYZ
    glm::vec3 Normal;       // Outward-facing surface normal (unit vector)
    glm::vec2 TexCoord;     // UV coordinates (0.0–1.0 range)
};

class Mesh {
public:
    // =========================================================================
    // Constructor — Upload geometry to GPU. Call once at construction.
    // =========================================================================
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<unsigned int>& indices);

    // --- Prevent accidental copy (OpenGL objects can't be trivially copied) ---
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // --- Allow move (so we can store Meshes in vectors / unique_ptrs) ---
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // =========================================================================
    // Destructor — Calls Release() to free GPU resources
    // =========================================================================
    ~Mesh();

    // =========================================================================
    // Draw — Bind VAO and issue a draw call
    // =========================================================================
    void Draw() const;

    // =========================================================================
    // Accessors for debug/stats
    // =========================================================================
    unsigned int GetVertexCount() const { return m_VertexCount; }
    unsigned int GetIndexCount()  const { return m_IndexCount; }

private:
    unsigned int m_VAO = 0;          // Vertex Array Object
    unsigned int m_VBO = 0;          // Vertex Buffer Object
    unsigned int m_EBO = 0;          // Element Buffer Object
    unsigned int m_VertexCount = 0;  // Number of vertices uploaded
    unsigned int m_IndexCount  = 0;  // Number of indices (elements to draw)

    // =========================================================================
    // SetupMesh — Creates VAO/VBO/EBO and configures attribute pointers
    // =========================================================================
    void SetupMesh(const std::vector<Vertex>& vertices,
                   const std::vector<unsigned int>& indices);

    // =========================================================================
    // Release — Delete GPU resources (called by destructor and move assignment)
    // =========================================================================
    void Release();
};
