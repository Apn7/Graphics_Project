// =============================================================================
// TextureManager.h — Texture Registry Singleton (Phase 6)
// =============================================================================
// Load-once, reuse-everywhere. Stores textures by string key.
// =============================================================================

#pragma once

#include "renderer/Texture.h"
#include "utils/Logger.h"

#include <unordered_map>
#include <memory>
#include <string>

class TextureManager {
public:
    // =========================================================================
    // Singleton access
    // =========================================================================
    static TextureManager& Get() {
        static TextureManager instance;
        return instance;
    }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // =========================================================================
    // Load — Load a texture from disk and store under a key.
    // Returns the GL texture ID. If already loaded, returns existing ID.
    // =========================================================================
    unsigned int Load(const std::string& key, const std::string& filepath) {
        auto it = m_Textures.find(key);
        if (it != m_Textures.end()) {
            return it->second->GetID();
        }

        auto tex = std::make_unique<Texture>(filepath);
        unsigned int id = tex->GetID();
        m_Textures[key] = std::move(tex);
        return id;
    }

    // =========================================================================
    // GetID — Returns GL texture ID by key (0 if not found)
    // =========================================================================
    unsigned int GetID(const std::string& key) const {
        auto it = m_Textures.find(key);
        return (it != m_Textures.end()) ? it->second->GetID() : 0;
    }

    // =========================================================================
    // Bind — Bind a texture by key to a texture unit slot
    // =========================================================================
    void Bind(const std::string& key, unsigned int slot = 0) {
        auto it = m_Textures.find(key);
        if (it != m_Textures.end()) {
            it->second->Bind(slot);
        }
    }

    // =========================================================================
    // LoadAll — Load all project textures at startup
    // =========================================================================
    void LoadAll() {
        Load("floor",   "textures/floor_tiles.jpg");
        Load("wall",    "textures/wall_plaster.jpg");
        Load("wood",    "textures/wood_dark.jpg");
        Load("ceiling", "textures/ceiling.jpg");
        Load("earth",   "textures/earth.png");
        Load("door",    "textures/door.jpg");

        LOG_INFO("TextureManager: All textures loaded.");
    }

private:
    TextureManager() = default;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
};
