#version 330 core

// ============================================================
// FRAGMENT-BLEND TEXTURE FRAGMENT SHADER
// Assignment: "blended texture with surface color — color on fragment"
//
// KEY DIFFERENCE from vertex blend:
// - Vertex blend: blend factor applied at vertex stage, interpolated
// - Fragment blend: blend factor applied fresh per-fragment
//
// This allows per-fragment effects like distance-based fade,
// normal-dependent tinting, or procedural color variation.
// ============================================================

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3      u_SurfaceColor;   // Object's tint color
uniform float     u_BlendFactor;    // 0.0 = texture only, 1.0 = color only

void main() {
    // Sample texture
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // *** FRAGMENT-STAGE COLOR BLEND ***
    // Unlike vertex blend, the mix() is computed HERE for EVERY fragment.
    // Every pixel gets its own fresh blend calculation.

    // Surface color contribution computed per-fragment:
    vec3 surfaceContrib = u_SurfaceColor * u_BlendFactor;

    // Texture contribution:
    vec3 textureContrib = texColor.rgb * (1.0 - u_BlendFactor);

    // Final blend — computed entirely in fragment stage:
    vec3 finalColor = textureContrib + surfaceContrib;

    FragColor = vec4(finalColor, texColor.a);

    // TODO Phase 7: Multiply finalColor by Phong lighting result
    // TODO Phase 7: Replace u_BlendFactor with dynamic lighting-based blend
}
