// =============================================================================
// Mesh.cpp — VAO/VBO/EBO Implementation (Phase 3)
// =============================================================================
// Implements the Mesh class: sets up OpenGL buffer objects, configures vertex
// attribute pointers, and handles rendering and cleanup.
//
// Phase 3 Update: Renamed Delete→Release, tracks vertex count, cleanup.
// =============================================================================

#include "renderer/Mesh.h"
#include "utils/Logger.h"

#include <glad/glad.h>      // OpenGL function pointers

// =============================================================================
// Constructor — Creates mesh from vertex and index data
// =============================================================================
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_VAO(0), m_VBO(0), m_EBO(0),
      m_VertexCount(static_cast<unsigned int>(vertices.size())),
      m_IndexCount(static_cast<unsigned int>(indices.size()))
{
    SetupMesh(vertices, indices);
    LOG_INFO("Mesh created: " + std::to_string(m_VertexCount) + " vertices, "
             + std::to_string(m_IndexCount) + " indices");
}

// =============================================================================
// Move Constructor — Transfers ownership of GPU resources
// =============================================================================
Mesh::Mesh(Mesh&& other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO),
      m_VertexCount(other.m_VertexCount), m_IndexCount(other.m_IndexCount)
{
    // Nullify the source so its destructor doesn't delete our resources
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_VertexCount = 0;
    other.m_IndexCount = 0;
}

// =============================================================================
// Move Assignment — Transfers ownership of GPU resources
// =============================================================================
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Free our current resources
        Release();

        // Take ownership of the other mesh's resources
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;
        m_VertexCount = other.m_VertexCount;
        m_IndexCount = other.m_IndexCount;

        // Nullify the source
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_VertexCount = 0;
        other.m_IndexCount = 0;
    }
    return *this;
}

// =============================================================================
// Destructor — Frees GPU resources
// =============================================================================
Mesh::~Mesh() {
    Release();
}

// =============================================================================
// Draw — Bind VAO and issue a draw call
// =============================================================================
void Mesh::Draw() const {
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    // No unbind — unnecessary state change; next draw will rebind its own VAO
}

// =============================================================================
// Release — Delete GPU resources (VAO, VBO, EBO)
// =============================================================================
// Resets IDs to 0 so a moved-from object's destructor is safe to call.
// =============================================================================
void Mesh::Release() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
}

// =============================================================================
// SetupMesh — Creates and configures all OpenGL buffer objects
// =============================================================================
void Mesh::SetupMesh(const std::vector<Vertex>& vertices,
                     const std::vector<unsigned int>& indices)
{
    // ---- Step 1: Generate OpenGL objects ----
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    // ---- Step 2: Bind the VAO (captures all subsequent state) ----
    glBindVertexArray(m_VAO);

    // ---- Step 3: Upload vertex data to VBO ----
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // ---- Step 4: Upload index data to EBO ----
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    // ---- Step 5: Configure vertex attribute pointers ----
    // Must match the layout qualifiers in the vertex shader.

    // Attribute 0: Position (vec3)
    // layout(location = 0) in vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);

    // Attribute 1: Normal (vec3)
    // layout(location = 1) in vec3 aNormal;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    // Attribute 2: TexCoord (vec2)
    // layout(location = 2) in vec2 aTexCoord;
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, TexCoord));
    glEnableVertexAttribArray(2);

    // TODO Phase 7: When textures are added, ensure TexCoord UVs tile correctly on large surfaces

    // ---- Step 6: Unbind VAO ----
    glBindVertexArray(0);
}
