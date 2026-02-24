// =============================================================================
// Shader.h — Shader Program Manager (Phase 2)
// =============================================================================
// Handles loading, compiling, and linking GLSL vertex and fragment shaders
// into an OpenGL shader program. Provides cached uniform setters for all
// common types. The uniform cache avoids repeated glGetUniformLocation calls.
//
// Usage:
//   Shader shader("shaders/basic.vert", "shaders/basic.frag");
//   shader.Use();
//   shader.SetMat4("u_Model", modelMatrix);
//   shader.SetVec3("u_Color", glm::vec3(0.45f, 0.28f, 0.10f));
// =============================================================================

#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>          // glm::vec2, vec3, vec4, mat3, mat4

class Shader {
public:
    // =========================================================================
    // Constructor — Loads, compiles, and links a vertex + fragment shader pair
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
    // Unbind — Deactivates any shader program (good practice, optional)
    // =========================================================================
    void Unbind() const;

    // =========================================================================
    // GetID — Returns the raw OpenGL shader program ID
    // =========================================================================
    unsigned int GetID() const { return m_ProgramID; }

    // =========================================================================
    // Uniform Setters — Set shader uniform variables by name
    // =========================================================================
    // Each method finds the location via GetUniformLocation (cached), then
    // uploads the value. If the uniform is not found, a warning is logged
    // once (not every frame).
    // =========================================================================
    void SetBool (const std::string& name, bool value)              const;
    void SetInt  (const std::string& name, int value)               const;
    void SetFloat(const std::string& name, float value)             const;
    void SetVec2 (const std::string& name, const glm::vec2& value)  const;
    void SetVec3 (const std::string& name, const glm::vec3& value)  const;
    void SetVec4 (const std::string& name, const glm::vec4& value)  const;
    void SetMat3 (const std::string& name, const glm::mat3& value)  const;
    void SetMat4 (const std::string& name, const glm::mat4& value)  const;

private:
    unsigned int m_ProgramID;   // OpenGL handle to the linked shader program

    // ---- Uniform Location Cache ----
    // Avoids repeated glGetUniformLocation calls each frame.
    // Marked mutable so const setter methods can still populate the cache.
    mutable std::unordered_map<std::string, int> m_UniformCache;

    // =========================================================================
    // GetUniformLocation — Returns the cached location of a uniform by name
    // =========================================================================
    // Checks cache first. If not found, queries OpenGL and stores the result.
    // Logs LOG_WARN once if the uniform doesn't exist (location = -1).
    // =========================================================================
    int GetUniformLocation(const std::string& name) const;

    // =========================================================================
    // CompileShader — Compiles a single shader stage (vertex or fragment)
    // =========================================================================
    unsigned int CompileShader(unsigned int type, const std::string& source);

    // =========================================================================
    // LinkProgram — Links compiled vertex and fragment shaders into a program
    // =========================================================================
    void LinkProgram(unsigned int vertID, unsigned int fragID);

    // =========================================================================
    // ReadFile — Reads entire file contents as a string
    // =========================================================================
    std::string ReadFile(const std::string& path) const;

    // =========================================================================
    // CheckCompileErrors — Checks for and logs shader compilation/link errors
    // =========================================================================
    void CheckCompileErrors(unsigned int shader, const std::string& type) const;

    // TODO future: Consider UBO (Uniform Buffer Objects) for shared matrices across shaders
};
