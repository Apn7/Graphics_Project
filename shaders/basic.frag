// =============================================================================
// BASIC (FLAT COLOR) FRAGMENT SHADER — Phase 7 Phong Upgrade
// =============================================================================
// Implements full Phong shading (ambient + diffuse + specular) for flat-color
// objects. Replaces the former solid-color output entirely.
// Light data arrives via uniforms set by Scene::SetLighting() each frame.
// =============================================================================

#version 330 core

// ---- Inputs from Vertex Shader ----
in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;   // Unused — kept for VAO layout compatibility

// ---- Output ----
out vec4 FragColor;

// ---- Material ----
uniform vec3  u_Color;
uniform float u_Alpha;
uniform float u_Shininess;
uniform float u_AmbientStrength;
uniform float u_SpecularStrength;

// ---- Global toggles ----
uniform bool u_LightsOn;
uniform bool u_AmbientOn;
uniform bool u_DiffuseOn;
uniform bool u_SpecularOn;
uniform vec3 u_ViewPos;

// ---- Directional Light ----
uniform bool  u_DirLightOn;
uniform vec3  u_DirLightDir;
uniform vec3  u_DirLightColor;
uniform float u_DirLightIntensity;

// ---- Point Lights ----
uniform bool  u_PointLightsOn;
uniform vec3  u_PointLightPos[6];
uniform vec3  u_PointLightColor[6];
uniform float u_PointConstant;
uniform float u_PointLinear;
uniform float u_PointQuadratic;

// =============================================================================
// CalcDirLight — Diffuse + Specular contribution from the directional light
// =============================================================================
vec3 CalcDirLight(vec3 norm, vec3 viewDir, vec3 baseColor) {
    if (!u_DirLightOn) return vec3(0.0);

    vec3 lightDir = normalize(-u_DirLightDir);

    float diff = u_DiffuseOn ? max(dot(norm, lightDir), 0.0) : 0.0;
    vec3 diffuse = diff * u_DirLightColor * u_DirLightIntensity * baseColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = u_SpecularOn ? pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess) : 0.0;
    vec3 specular = spec * u_DirLightColor * u_DirLightIntensity * u_SpecularStrength;

    return diffuse + specular;
}

// =============================================================================
// CalcPointLight — Diffuse + Specular from one attenuated point light
// =============================================================================
vec3 CalcPointLight(int i, vec3 norm, vec3 viewDir, vec3 baseColor) {
    vec3  lightDir    = normalize(u_PointLightPos[i] - v_FragPos);
    float distance    = length(u_PointLightPos[i] - v_FragPos);
    float attenuation = 1.0 / (u_PointConstant
                              + u_PointLinear    * distance
                              + u_PointQuadratic * distance * distance);

    float diff = u_DiffuseOn ? max(dot(norm, lightDir), 0.0) : 0.0;
    vec3 diffuse = diff * u_PointLightColor[i] * baseColor * attenuation;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = u_SpecularOn ? pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess) : 0.0;
    vec3 specular = spec * u_PointLightColor[i] * attenuation * u_SpecularStrength;

    return diffuse + specular;
}

void main() {
    vec3 baseColor = u_Color;

    if (!u_LightsOn) {
        // Lights globally off — render flat color so the scene is still visible
        FragColor = vec4(baseColor * 0.35, u_Alpha);
        return;
    }

    vec3 norm    = normalize(v_Normal);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    // Ambient
    vec3 result = u_AmbientOn
        ? (u_AmbientStrength * baseColor)
        : vec3(0.0);

    // Directional light
    result += CalcDirLight(norm, viewDir, baseColor);

    // Point lights
    if (u_PointLightsOn) {
        result += CalcPointLight(0, norm, viewDir, baseColor);
        result += CalcPointLight(1, norm, viewDir, baseColor);
        result += CalcPointLight(2, norm, viewDir, baseColor);
        result += CalcPointLight(3, norm, viewDir, baseColor);
        result += CalcPointLight(4, norm, viewDir, baseColor);
        result += CalcPointLight(5, norm, viewDir, baseColor);
    }

    FragColor = vec4(clamp(result, 0.0, 1.0), u_Alpha);
}
