#version 330 core

// ============================================================
// VERTEX-BLEND TEXTURE VERTEX SHADER
// Assignment: "blended texture with surface color — color on vertex"
// KEY CONCEPT: The blend between texture and surface color is
// computed HERE in the vertex shader. The blended color is
// passed to the fragment shader as a varying.
//
// This means the blend interpolates across the triangle surface.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Output: NOT just UV — we pass the ALREADY-BLENDED color to the fragment
out vec2  v_TexCoord;
out vec3  v_BlendedColor;   // ← "color computed on vertex"
out vec3  v_FragPos;
out vec3  v_Normal;

uniform mat4  u_Model;
uniform mat4  u_View;
uniform mat4  u_Projection;
uniform float u_TileX;
uniform float u_TileY;

// Surface color (e.g. wall cream — set from C++ per object)
uniform vec3  u_SurfaceColor;

// How much of the surface color to mix in (0.0 = all texture, 1.0 = all color)
uniform float u_BlendFactor;

void main() {
    v_FragPos  = vec3(u_Model * vec4(aPos, 1.0));
    v_Normal   = mat3(transpose(inverse(u_Model))) * aNormal;
    v_TexCoord = aTexCoord * vec2(u_TileX, u_TileY);

    // *** VERTEX-STAGE COLOR BLEND ***
    // We cannot sample the texture here (no sampler in vertex shaders).
    // So we pre-compute the surface color contribution at the vertex.
    //
    // v_BlendedColor = mix(white, surfaceColor, blendFactor)
    // Then in fragment: FragColor = texture * v_BlendedColor
    //
    // When blend = 0.0: v_BlendedColor = (1,1,1) → texture × 1 = pure texture
    // When blend = 1.0: v_BlendedColor = u_SurfaceColor → texture tinted fully
    v_BlendedColor = mix(vec3(1.0), u_SurfaceColor, u_BlendFactor);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
