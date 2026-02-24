// =============================================================================
// Renderer.h — Central Render Utilities (Phase 1 Stub)
// =============================================================================
// Provides static rendering utility functions. In Phase 1, only the screen
// clear function is implemented. Future phases will add draw dispatching
// with transform support.
//
// Usage:
//   Renderer::Clear(0.05f, 0.07f, 0.12f);  // Dark navy background
// =============================================================================

#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>      // OpenGL function pointers

class Renderer {
public:
    // =========================================================================
    // Clear — Clears the screen with the specified color
    // =========================================================================
    // Parameters:
    //   r, g, b — red, green, blue color components (0.0 to 1.0)
    //   a       — alpha component (default: 1.0 = fully opaque)
    // =========================================================================
    static void Clear(float r, float g, float b, float a = 1.0f) {
        // Set the clear color (the color used to fill the screen)
        glClearColor(r, g, b, a);

        // Clear both the color buffer and the depth buffer
        // GL_COLOR_BUFFER_BIT: resets pixel colors to the clear color
        // GL_DEPTH_BUFFER_BIT: resets depth values (needed for depth testing)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // TODO Phase 5: static void Draw(Mesh& mesh, Shader& shader, const glm::mat4& transform);
    // TODO Phase 8: Add instanced drawing support
};

#endif // RENDERER_H
