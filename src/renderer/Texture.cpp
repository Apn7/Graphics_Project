// =============================================================================
// Texture.cpp — OpenGL Texture Implementation (Phase 6)
// =============================================================================
// IMPORTANT: #define STB_IMAGE_IMPLEMENTATION must appear ONCE in the entire
// project. It is defined here and nowhere else.
// =============================================================================

#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"

#include "renderer/Texture.h"
#include "utils/Logger.h"

#include <glad/glad.h>

// =============================================================================
// Constructor — Load image and create OpenGL texture
// =============================================================================
Texture::Texture(const std::string& filepath, int wrapMode, int filterMode) {
    // OpenGL expects origin at bottom-left; most images have origin at top-left
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filepath.c_str(),
                                    &m_Width, &m_Height, &m_Channels, 0);

    if (!data) {
        LOG_ERROR("Texture failed to load: " + filepath);
        return;
    }

    // Determine pixel format from channel count
    GLenum format = (m_Channels == 4) ? GL_RGBA :
                    (m_Channels == 3) ? GL_RGB  : GL_RED;

    // Create texture object
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    // Upload pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0,
                 format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Wrapping — GL_REPEAT so tiled textures work
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // Filtering — trilinear for minification, user choice for magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

    // Free CPU-side image data
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_INFO("Texture loaded: " + filepath +
             " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) +
             ", " + std::to_string(m_Channels) + " channels)");
}

// =============================================================================
// Destructor
// =============================================================================
Texture::~Texture() {
    if (m_ID) glDeleteTextures(1, &m_ID);
}

// =============================================================================
// Move constructor / assignment
// =============================================================================
Texture::Texture(Texture&& other) noexcept
    : m_ID(other.m_ID), m_Width(other.m_Width),
      m_Height(other.m_Height), m_Channels(other.m_Channels)
{
    other.m_ID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_ID) glDeleteTextures(1, &m_ID);
        m_ID       = other.m_ID;
        m_Width    = other.m_Width;
        m_Height   = other.m_Height;
        m_Channels = other.m_Channels;
        other.m_ID = 0;
    }
    return *this;
}

// =============================================================================
// Bind / Unbind
// =============================================================================
void Texture::Bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
