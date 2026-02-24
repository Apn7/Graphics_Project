// =============================================================================
// Mesh.cpp — VAO/VBO/EBO Implementation
// =============================================================================
// Implements the Mesh class: sets up OpenGL buffer objects, configures vertex
// attribute pointers, and handles rendering and cleanup.
// =============================================================================

#include "renderer/Mesh.h"
#include "utils/Logger.h"

#include <glad/glad.h>      // OpenGL function pointers

// =============================================================================
// Constructor — Creates mesh from vertex and index data
// =============================================================================
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_VAO(0), m_VBO(0), m_EBO(0), m_IndexCount(static_cast<unsigned int>(indices.size()))
{
    SetupMesh(vertices, indices);
    LOG_INFO("Mesh created with " + std::to_string(vertices.size()) + " vertices, "
             + std::to_string(indices.size()) + " indices");
}

// =============================================================================
// Destructor — Automatically frees GPU resources
// =============================================================================
Mesh::~Mesh() {
    Delete();
}

// =============================================================================
// Move Constructor — Transfers ownership of GPU resources
// =============================================================================
Mesh::Mesh(Mesh&& other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO),
      m_EBO(other.m_EBO), m_IndexCount(other.m_IndexCount)
{
    // Nullify the source so its destructor doesn't delete our resources
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_IndexCount = 0;
}

// =============================================================================
// Move Assignment — Transfers ownership of GPU resources
// =============================================================================
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Free our current resources
        Delete();

        // Take ownership of the other mesh's resources
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;
        m_IndexCount = other.m_IndexCount;

        // Nullify the source
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_IndexCount = 0;
    }
    return *this;
}

// =============================================================================
// Draw — Renders the mesh
// =============================================================================
// Binds the Vertex Array Object and calls glDrawElements to draw all
// indexed triangles. The currently bound shader determines how the
// vertices are transformed and colored.
// =============================================================================
void Mesh::Draw() const {
    glBindVertexArray(m_VAO);       // Bind the VAO (activates all attribute pointers)
    glDrawElements(
        GL_TRIANGLES,               // Drawing mode: triangles
        m_IndexCount,               // Number of indices to draw
        GL_UNSIGNED_INT,            // Data type of indices
        0                           // Offset into the EBO (start from beginning)
    );
    glBindVertexArray(0);           // Unbind VAO to prevent accidental modification
}

// =============================================================================
// Delete — Frees all GPU-allocated memory
// =============================================================================
void Mesh::Delete() {
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
// This is where the actual GPU setup happens:
//   1. Generate VAO, VBO, EBO
//   2. Upload vertex data to VBO
//   3. Upload index data to EBO
//   4. Configure vertex attribute pointers (position, color)
// =============================================================================
void Mesh::SetupMesh(const std::vector<Vertex>& vertices,
                     const std::vector<unsigned int>& indices)
{
    // ---- Step 1: Generate OpenGL objects ----
    glGenVertexArrays(1, &m_VAO);   // Create the Vertex Array Object
    glGenBuffers(1, &m_VBO);        // Create the Vertex Buffer Object
    glGenBuffers(1, &m_EBO);        // Create the Element Buffer Object

    // ---- Step 2: Bind the VAO ----
    // All subsequent VBO, EBO, and attribute pointer calls are "captured" by the VAO.
    glBindVertexArray(m_VAO);

    // ---- Step 3: Upload vertex data to VBO ----
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,                            // Target: vertex buffer
        vertices.size() * sizeof(Vertex),            // Size in bytes
        vertices.data(),                             // Pointer to the data
        GL_STATIC_DRAW                               // Usage hint: data won't change
    );

    // ---- Step 4: Upload index data to EBO ----
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,                     // Target: index buffer
        indices.size() * sizeof(unsigned int),        // Size in bytes
        indices.data(),                               // Pointer to the data
        GL_STATIC_DRAW                                // Usage hint: data won't change
    );

    // ---- Step 5: Configure vertex attribute pointers ----
    // These tell OpenGL how to interpret the data in the VBO.

    // Attribute 0: Position (vec3 = 3 floats)
    // Located at layout(location = 0) in the vertex shader
    glVertexAttribPointer(
        0,                                  // Attribute index (location = 0)
        3,                                  // Number of components (vec3 = 3)
        GL_FLOAT,                           // Data type
        GL_FALSE,                           // Don't normalize
        sizeof(Vertex),                     // Stride: bytes between consecutive vertices
        (void*)offsetof(Vertex, Position)   // Offset: where Position starts in Vertex struct
    );
    glEnableVertexAttribArray(0);           // Enable attribute 0

    // Attribute 1: Color (vec3 = 3 floats)
    // Located at layout(location = 1) in the vertex shader
    glVertexAttribPointer(
        1,                                  // Attribute index (location = 1)
        3,                                  // Number of components (vec3 = 3)
        GL_FLOAT,                           // Data type
        GL_FALSE,                           // Don't normalize
        sizeof(Vertex),                     // Stride: same as above
        (void*)offsetof(Vertex, Color)      // Offset: where Color starts in Vertex struct
    );
    glEnableVertexAttribArray(1);           // Enable attribute 1

    // TODO Phase 6: Add attribute 2 — Normal (vec3, location = 2)
    // TODO Phase 7: Add attribute 3 — TexCoord (vec2, location = 3)

    // ---- Step 6: Unbind ----
    // Unbind VBO (safe — VAO already captured the binding)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Do NOT unbind EBO while VAO is active — it would break the VAO's EBO reference
    // Unbind VAO last
    glBindVertexArray(0);
}
