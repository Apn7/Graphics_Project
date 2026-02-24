#version 330 core

// ============================================================
// FRAGMENT-BLEND TEXTURE VERTEX SHADER
// Assignment: "blended texture with surface color — color on fragment"
// KEY CONCEPT: Unlike the vertex blend shader, NO color computation
// happens here. We only pass raw data to the fragment shader.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 v_TexCoord;
out vec3 v_FragPos;    // World position (for fragment-stage calculations)
out vec3 v_Normal;

uniform mat4  u_Model;
uniform mat4  u_View;
uniform mat4  u_Projection;
uniform float u_TileX;
uniform float u_TileY;

void main() {
    v_FragPos  = vec3(u_Model * vec4(aPos, 1.0));
    v_Normal   = mat3(transpose(inverse(u_Model))) * aNormal;
    v_TexCoord = aTexCoord * vec2(u_TileX, u_TileY);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);

    // NOTE: No color computation here — that is the whole point of this shader.
    // The difference from texture_vertex_blend: blend factor applied in fragment.
}
