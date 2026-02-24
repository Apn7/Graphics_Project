// =============================================================================
// Texture.h — OpenGL Texture Wrapper (Phase 6)
// =============================================================================
// Loads image from disk using stb_image, creates GL texture with mipmaps.
// Supports GL_REPEAT wrap and GL_LINEAR filtering by default.
// =============================================================================

#pragma once
#include <string>

class Texture {
public:
    // Load from file. Flips image vertically (OpenGL expects bottom-left origin).
    Texture(const std::string& filepath,
            int wrapMode   = 0x2901,   // GL_REPEAT
            int filterMode = 0x2601);  // GL_LINEAR

    ~Texture();

    // Prevent copy, allow move
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Bind to a texture unit (0, 1, 2, ...)
    void Bind(unsigned int unit = 0) const;
    void Unbind() const;

    unsigned int GetID()     const { return m_ID; }
    int          GetWidth()  const { return m_Width; }
    int          GetHeight() const { return m_Height; }
    bool         IsLoaded()  const { return m_ID != 0; }

private:
    unsigned int m_ID       = 0;
    int          m_Width    = 0;
    int          m_Height   = 0;
    int          m_Channels = 0;
};
