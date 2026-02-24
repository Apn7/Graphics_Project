#version 330 core

// ============================================================
// SIMPLE TEXTURE VERTEX SHADER
// Assignment: "simple texture without surface color"
// No color calculation here — just pass UV to fragment shader.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

// UV tiling: repeat texture N times across the surface
uniform float u_TileX;
uniform float u_TileY;

void main() {
    v_FragPos   = vec3(u_Model * vec4(aPos, 1.0));
    v_Normal    = mat3(transpose(inverse(u_Model))) * aNormal;

    // Apply tiling multiplier so texture repeats instead of stretching
    v_TexCoord  = aTexCoord * vec2(u_TileX, u_TileY);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
