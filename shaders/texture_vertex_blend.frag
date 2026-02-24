#version 330 core

// ============================================================
// VERTEX-BLEND TEXTURE FRAGMENT SHADER
// The blending was already computed in the vertex shader.
// This shader just applies the pre-computed blend to the texture.
// ============================================================

in vec2  v_TexCoord;
in vec3  v_BlendedColor;   // Received from vertex shader (interpolated across triangle)
in vec3  v_FragPos;
in vec3  v_Normal;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    // Sample the texture
    vec4 texColor = texture(u_Texture, v_TexCoord);

    // Apply the vertex-computed blend color.
    // Since v_BlendedColor was computed per-vertex and interpolated,
    // this is technically a vertex-stage color operation applied in the fragment.
    vec3 finalColor = texColor.rgb * v_BlendedColor;

    FragColor = vec4(finalColor, texColor.a);

    // TODO Phase 7: Apply Phong lighting to finalColor before output
}
