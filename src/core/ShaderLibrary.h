// =============================================================================
// ShaderLibrary.h — Shader Management Singleton (Phase 2)
// =============================================================================
// Manages named shader programs. Provides a central place to load, store,
// and retrieve shaders by name. Uses the singleton pattern so any part
// of the codebase can access shaders without passing them around.
//
// Usage:
//   ShaderLibrary::Get().LoadAll();
//   Shader& basic = ShaderLibrary::Get().GetBasic();
//   basic.Use();
// =============================================================================

#pragma once

#include "core/Shader.h"

#include <string>
#include <unordered_map>
#include <memory>               // std::unique_ptr for ownership

class ShaderLibrary {
public:
    // =========================================================================
    // Get — Returns the singleton instance
    // =========================================================================
    static ShaderLibrary& Get() {
        static ShaderLibrary instance;  // Created once, lives until program exit
        return instance;
    }

    // --- Prevent copying/moving the singleton ---
    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;

    // =========================================================================
    // LoadAll — Loads all shaders needed by the current phase
    // =========================================================================
    // Call this once at startup after the OpenGL context is created.
    // =========================================================================
    void LoadAll();

    // =========================================================================
    // Load — Loads a single shader program and stores it under the given name
    // =========================================================================
    void Load(const std::string& name,
              const std::string& vertexPath,
              const std::string& fragmentPath);

    // =========================================================================
    // GetShader — Returns a reference to a named shader
    // =========================================================================
    // Throws std::out_of_range if the name doesn't exist.
    // =========================================================================
    Shader& GetShader(const std::string& name);

    // =========================================================================
    // GetBasic — Convenience getter for the "basic" shader
    // =========================================================================
    Shader& GetBasic();

private:
    // Private constructor — only Get() can create the instance
    ShaderLibrary() = default;

    // Storage: shader name → unique_ptr<Shader>
    // unique_ptr ensures automatic cleanup when the library is destroyed
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_Shaders;
};
