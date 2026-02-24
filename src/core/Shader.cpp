// =============================================================================
// Shader.cpp — Shader Program Implementation
// =============================================================================
// Implements shader loading, compilation, linking, error checking, and
// uniform variable setters for the OpenGL 3.3 Core Profile.
// =============================================================================

#include "core/Shader.h"
#include "utils/Logger.h"
#include "utils/FileUtils.h"

#include <glad/glad.h>              // OpenGL function pointers
#include <glm/gtc/type_ptr.hpp>     // glm::value_ptr — converts glm types to raw pointers

// =============================================================================
// Constructor — Loads shader files, compiles them, and links into a program
// =============================================================================
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : m_ProgramID(0)
{
    // ---- Step 1: Read shader source code from files ----
    std::string vertexSource = FileUtils::ReadFile(vertexPath);
    std::string fragmentSource = FileUtils::ReadFile(fragmentPath);

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
    unsigned int vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);

    // If either compilation failed, abort
    if (vertexShader == 0 || fragmentShader == 0) {
        LOG_ERROR("Shader compilation failed — program will not be created");
        // Clean up any successfully compiled shader
        if (vertexShader != 0)   glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return;
    }

    // ---- Step 3: Create the shader program and link both stages ----
    m_ProgramID = glCreateProgram();
    glAttachShader(m_ProgramID, vertexShader);      // Attach vertex stage
    glAttachShader(m_ProgramID, fragmentShader);    // Attach fragment stage
    glLinkProgram(m_ProgramID);                     // Link into a complete program

    // Check for linking errors
    CheckLinkErrors(m_ProgramID);

    // ---- Step 4: Clean up individual shader objects ----
    // After linking, the individual shader objects are no longer needed.
    // The linked program contains all the compiled code.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_INFO("Shader program created successfully (ID: " + std::to_string(m_ProgramID) + ")");
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
// Use — Activates this shader program
// =============================================================================
// All subsequent draw calls and uniform sets will use this program.
// =============================================================================
void Shader::Use() const {
    glUseProgram(m_ProgramID);
}

// =============================================================================
// Uniform Setters — Upload values to shader uniform variables
// =============================================================================
// glGetUniformLocation finds the uniform's location in the shader program.
// If the uniform doesn't exist or is optimized away, location = -1 and
// the gl call is silently ignored.
// =============================================================================

void Shader::SetInt(const std::string& name, int value) const {
    GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
    glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
    glUniform1f(location, value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
    // glUniform3fv: upload 1 vec3, using a pointer to the raw float data
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
    // glUniformMatrix4fv: upload 1 mat4, GL_FALSE = don't transpose, raw pointer
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

// =============================================================================
// GetID — Returns the OpenGL program ID
// =============================================================================
unsigned int Shader::GetID() const {
    return m_ProgramID;
}

// =============================================================================
// CompileShader — Compiles a single GLSL shader stage
// =============================================================================
unsigned int Shader::CompileShader(const std::string& source, unsigned int type) const {
    // Create a shader object of the specified type
    unsigned int shader = glCreateShader(type);

    // Upload the source code to the shader object
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);

    // Compile the shader
    glCompileShader(shader);

    // Determine the shader type name for logging
    std::string typeName = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        CheckCompileErrors(shader, typeName);
        glDeleteShader(shader);
        return 0;   // Return 0 to indicate failure
    }

    LOG_INFO(typeName + " shader compiled successfully");
    return shader;
}

// =============================================================================
// CheckCompileErrors — Logs shader compilation error details
// =============================================================================
void Shader::CheckCompileErrors(unsigned int shader, const std::string& type) const {
    // Get the length of the error log
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    // Retrieve and print the error log
    if (logLength > 0) {
        std::string infoLog(logLength, '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, &infoLog[0]);
        LOG_ERROR(type + " shader compilation failed:\n" + infoLog);
    } else {
        LOG_ERROR(type + " shader compilation failed (no error log available)");
    }
}

// =============================================================================
// CheckLinkErrors — Logs shader program linking error details
// =============================================================================
void Shader::CheckLinkErrors(unsigned int program) const {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        // Get the length of the error log
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {
            std::string infoLog(logLength, '\0');
            glGetProgramInfoLog(program, logLength, nullptr, &infoLog[0]);
            LOG_ERROR("Shader program linking failed:\n" + infoLog);
        } else {
            LOG_ERROR("Shader program linking failed (no error log available)");
        }
    }
}
