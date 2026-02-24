// =============================================================================
// Shader.cpp — Shader Program Implementation (Phase 2)
// =============================================================================
// Implements the production-ready shader system with:
//   - File reading (built-in, no external dependency)
//   - Shader compilation with detailed error reporting
//   - Program linking with error checking
//   - Cached uniform location lookups (warn-once for missing uniforms)
//   - Full set of uniform setters for bool, int, float, vec2-4, mat3-4
// =============================================================================

#include "core/Shader.h"
#include "utils/Logger.h"

#include <glad/glad.h>              // OpenGL function pointers
#include <glm/gtc/type_ptr.hpp>     // glm::value_ptr — converts glm types to raw float pointers

#include <fstream>                  // std::ifstream — file reading
#include <sstream>                  // std::stringstream — buffer file contents

// =============================================================================
// Constructor — Reads shader files, compiles them, and links into a program
// =============================================================================
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : m_ProgramID(0)
{
    // ---- Step 1: Read shader source code from files ----
    std::string vertexSource = ReadFile(vertexPath);
    std::string fragmentSource = ReadFile(fragmentPath);

    // Check that both files were read successfully
    if (vertexSource.empty()) {
        LOG_ERROR("Vertex shader source is empty — check file path: " + vertexPath);
        return;
    }
    if (fragmentSource.empty()) {
        LOG_ERROR("Fragment shader source is empty — check file path: " + fragmentPath);
        return;
    }

    // ---- Step 2: Compile each shader stage ----
    unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // If either compilation failed, abort
    if (vertexShader == 0 || fragmentShader == 0) {
        LOG_ERROR("Shader compilation failed — program will not be created");
        if (vertexShader != 0)   glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return;
    }

    // ---- Step 3: Link into a complete shader program ----
    LinkProgram(vertexShader, fragmentShader);
}

// =============================================================================
// Destructor — Deletes the shader program from GPU memory
// =============================================================================
Shader::~Shader() {
    if (m_ProgramID != 0) {
        glDeleteProgram(m_ProgramID);
        LOG_INFO("Shader program deleted (ID: " + std::to_string(m_ProgramID) + ")");
    }
}

// =============================================================================
// Use — Activates this shader program for rendering
// =============================================================================
void Shader::Use() const {
    glUseProgram(m_ProgramID);
}

// =============================================================================
// Unbind — Deactivates any active shader program
// =============================================================================
// Sets the current program to 0 (none). Good practice between draw calls
// when switching shaders, but optional in tight render loops.
// =============================================================================
void Shader::Unbind() const {
    glUseProgram(0);
}

// =============================================================================
// Uniform Setters — All use GetUniformLocation (cached) then glUniform*
// =============================================================================

void Shader::SetBool(const std::string& name, bool value) const {
    // OpenGL has no bool uniform — use int (0 = false, 1 = true)
    glUniform1i(GetUniformLocation(name), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value) const {
    // GL_FALSE = don't transpose the matrix (GLM already uses column-major)
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

// =============================================================================
// GetUniformLocation — Returns the cached uniform location
// =============================================================================
// First checks the cache. If not found, queries OpenGL and caches the result.
// If the uniform doesn't exist in the shader (location == -1), logs a warning
// ONCE — the warning is not repeated on subsequent lookups because the -1
// value is cached.
// =============================================================================
int Shader::GetUniformLocation(const std::string& name) const {
    // Check cache first — avoids calling glGetUniformLocation every frame
    auto it = m_UniformCache.find(name);
    if (it != m_UniformCache.end()) {
        return it->second;  // Return cached location
    }

    // Not in cache — query OpenGL for the location
    int location = glGetUniformLocation(m_ProgramID, name.c_str());

    // Warn once if the uniform was not found
    // After caching -1, this warning won't fire again for the same name
    if (location == -1) {
        LOG_WARN("Uniform '" + name + "' not found in shader program (ID: "
                 + std::to_string(m_ProgramID) + ") — it may be unused/optimized out");
    }

    // Cache the result (even -1, so we don't warn again)
    m_UniformCache[name] = location;
    return location;
}

// =============================================================================
// ReadFile — Reads the entire file at the given path into a string
// =============================================================================
std::string Shader::ReadFile(const std::string& path) const {
    std::ifstream file(path);

    if (!file.is_open()) {
        LOG_ERROR("Failed to open shader file: " + path);
        return "";
    }

    // Read entire file into a stringstream buffer
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    LOG_INFO("Successfully read shader file: " + path);
    return buffer.str();
}

// =============================================================================
// CompileShader — Compiles a single GLSL shader stage
// =============================================================================
unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    // Determine shader type name for logging
    std::string typeName = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";

    // Create the shader object
    unsigned int shader = glCreateShader(type);

    // Upload source code
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);

    // Compile
    glCompileShader(shader);

    // Check for errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        CheckCompileErrors(shader, typeName);
        glDeleteShader(shader);
        return 0;   // Indicate failure
    }

    LOG_INFO(typeName + " shader compiled successfully");
    return shader;
}

// =============================================================================
// LinkProgram — Links vertex and fragment shaders into a complete program
// =============================================================================
void Shader::LinkProgram(unsigned int vertID, unsigned int fragID) {
    // Create the program and attach both shader stages
    m_ProgramID = glCreateProgram();
    glAttachShader(m_ProgramID, vertID);
    glAttachShader(m_ProgramID, fragID);

    // Link the program
    glLinkProgram(m_ProgramID);

    // Check for link errors
    GLint success;
    glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        CheckCompileErrors(m_ProgramID, "PROGRAM");
    }

    // Delete individual shader objects — they're linked into the program now
    glDeleteShader(vertID);
    glDeleteShader(fragID);

    LOG_INFO("Shader program linked successfully (ID: " + std::to_string(m_ProgramID) + ")");
}

// =============================================================================
// CheckCompileErrors — Logs compile or link errors with the full GLSL log
// =============================================================================
void Shader::CheckCompileErrors(unsigned int shader, const std::string& type) const {
    GLint logLength;

    if (type == "PROGRAM") {
        // ---- Link error ----
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string infoLog(logLength, '\0');
            glGetProgramInfoLog(shader, logLength, nullptr, &infoLog[0]);
            LOG_ERROR("Shader PROGRAM linking failed:\n" + infoLog);
        } else {
            LOG_ERROR("Shader PROGRAM linking failed (no error log available)");
        }
    } else {
        // ---- Compile error (VERTEX or FRAGMENT) ----
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string infoLog(logLength, '\0');
            glGetShaderInfoLog(shader, logLength, nullptr, &infoLog[0]);
            LOG_ERROR(type + " shader compilation failed:\n" + infoLog);
        } else {
            LOG_ERROR(type + " shader compilation failed (no error log available)");
        }
    }
}
