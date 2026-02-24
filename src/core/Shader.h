// =============================================================================
// Shader.h — Shader Program Manager
// =============================================================================
// Handles loading, compiling, and linking GLSL vertex and fragment shaders
// into an OpenGL shader program. Provides uniform setters for common types.
//
// Usage:
//   Shader shader("shaders/basic.vert", "shaders/basic.frag");
//   shader.Use();
//   shader.SetMat4("model", modelMatrix);
// =============================================================================

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp>          // glm::vec3, glm::mat4

class Shader {
public:
    // =========================================================================
    // Constructor — Loads, compiles, and links a vertex + fragment shader pair
    // =========================================================================
    // Parameters:
    //   vertexPath   — file path to the vertex shader (.vert)
    //   fragmentPath — file path to the fragment shader (.frag)
    // =========================================================================
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    // =========================================================================
    // Destructor — Deletes the shader program from GPU memory
    // =========================================================================
    ~Shader();

    // --- Prevent copying (shader programs are GPU resources) ---
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // =========================================================================
    // Use — Activates this shader program for subsequent draw calls
    // =========================================================================
    void Use() const;

    // =========================================================================
    // Uniform Setters — Set shader uniform variables by name
    // =========================================================================
    // Each method finds the uniform location by name and uploads the value.
    // If the uniform is not found, OpenGL silently ignores it (location = -1).
    // =========================================================================
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

    // =========================================================================
    // GetID — Returns the OpenGL shader program ID
    // =========================================================================
    unsigned int GetID() const;

private:
    unsigned int m_ProgramID;   // OpenGL handle to the linked shader program

    // =========================================================================
    // CompileShader — Compiles a single shader stage (vertex or fragment)
    // =========================================================================
    // Parameters:
    //   source — the GLSL source code as a string
    //   type   — GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
    // Returns:
    //   The compiled shader's OpenGL ID, or 0 on failure
    // =========================================================================
    unsigned int CompileShader(const std::string& source, unsigned int type) const;

    // =========================================================================
    // CheckCompileErrors — Checks for and logs shader compilation errors
    // =========================================================================
    void CheckCompileErrors(unsigned int shader, const std::string& type) const;

    // =========================================================================
    // CheckLinkErrors — Checks for and logs shader program linking errors
    // =========================================================================
    void CheckLinkErrors(unsigned int program) const;
};

#endif // SHADER_H
