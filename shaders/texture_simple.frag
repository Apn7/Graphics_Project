#version 330 core

// ============================================================
// SIMPLE TEXTURE FRAGMENT SHADER
// Assignment: "simple texture without surface color"
// Outputs ONLY the texture sample — zero color influence.
// This is the purest form of texturing.
// ============================================================

in vec2 v_TexCoord;
in vec3 v_FragPos;   // Available for Phase 7 lighting — unused now
in vec3 v_Normal;    // Available for Phase 7 lighting — unused now

out vec4 FragColor;

uniform sampler2D u_Texture;   // The bound texture unit

void main() {
    // Sample the texture at the interpolated UV coordinate.
    // No color blending, no surface color — pure texture output.
    vec4 texColor = texture(u_Texture, v_TexCoord);

    FragColor = texColor;

    // TODO Phase 7 (Lighting): FragColor = vec4(ApplyPhong(texColor.rgb, ...), 1.0);
}
