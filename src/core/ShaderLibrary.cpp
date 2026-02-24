// =============================================================================
// ShaderLibrary.cpp — Shader Library Implementation (Phase 2)
// =============================================================================
// Implements the ShaderLibrary singleton: loads, stores, and retrieves
// named shader programs.
// =============================================================================

#include "core/ShaderLibrary.h"
#include "utils/Logger.h"

#include <stdexcept>    // std::out_of_range

// =============================================================================
// LoadAll — Loads all shaders needed for the current phase
// =============================================================================
// In Phase 2, only the "basic" shader is needed. Future phases will add
// more shaders here (phong, unlit, etc.).
// =============================================================================
void ShaderLibrary::LoadAll() {
    // Phase 2-5: flat color shader
    Load("basic", "shaders/basic.vert", "shaders/basic.frag");

    // Phase 6: texture shaders
    Load("texture_simple",   "shaders/texture_simple.vert",        "shaders/texture_simple.frag");
    Load("texture_vertex",   "shaders/texture_vertex_blend.vert",  "shaders/texture_vertex_blend.frag");
    Load("texture_fragment", "shaders/texture_fragment_blend.vert", "shaders/texture_fragment_blend.frag");

    // TODO Phase 7 (Lighting): Load("phong", "shaders/phong.vert", "shaders/phong.frag");

    LOG_INFO("ShaderLibrary: All shaders loaded successfully.");
}

// =============================================================================
// Load — Creates a Shader from files and stores it under the given name
// =============================================================================
void ShaderLibrary::Load(const std::string& name,
                         const std::string& vertexPath,
                         const std::string& fragmentPath)
{
    // Check if a shader with this name already exists
    if (m_Shaders.find(name) != m_Shaders.end()) {
        LOG_WARN("ShaderLibrary: Shader '" + name + "' already loaded — skipping");
        return;
    }

    // Create the shader and store it
    auto shader = std::make_unique<Shader>(vertexPath, fragmentPath);

    // Verify the shader compiled/linked successfully
    if (shader->GetID() == 0) {
        LOG_ERROR("ShaderLibrary: Failed to load shader '" + name + "'");
        return;
    }

    m_Shaders[name] = std::move(shader);
    LOG_INFO("ShaderLibrary: Loaded shader '" + name + "' successfully");
}

// =============================================================================
// GetShader — Returns a reference to a named shader
// =============================================================================
Shader& ShaderLibrary::GetShader(const std::string& name) {
    auto it = m_Shaders.find(name);
    if (it == m_Shaders.end()) {
        LOG_ERROR("ShaderLibrary: Shader '" + name + "' not found!");
        throw std::out_of_range("ShaderLibrary: Shader '" + name + "' not found");
    }
    return *(it->second);
}

// =============================================================================
// GetBasic — Convenience getter for the "basic" shader
// =============================================================================
Shader& ShaderLibrary::GetBasic() {
    return GetShader("basic");
}

Shader& ShaderLibrary::GetTextureSimple() {
    return GetShader("texture_simple");
}

Shader& ShaderLibrary::GetTextureVertex() {
    return GetShader("texture_vertex");
}

Shader& ShaderLibrary::GetTextureFragment() {
    return GetShader("texture_fragment");
}
