// =============================================================================
// FRAGMENT-BLEND TEXTURE FRAGMENT SHADER — Phase 7 Phong Upgrade
// =============================================================================
// baseColor = per-fragment blend of texture + surface color
// Phong lighting applied on top of the blended result.
// =============================================================================
#version 330 core

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3      u_SurfaceColor;
uniform float     u_BlendFactor;
uniform float     u_TileX;
uniform float     u_TileY;

uniform float u_Shininess;
uniform float u_AmbientStrength;
uniform float u_SpecularStrength;

uniform bool u_LightsOn;
uniform bool u_AmbientOn;
uniform bool u_DiffuseOn;
uniform bool u_SpecularOn;
uniform vec3 u_ViewPos;

uniform bool  u_DirLightOn;
uniform vec3  u_DirLightDir;
uniform vec3  u_DirLightColor;
uniform float u_DirLightIntensity;

uniform bool  u_PointLightsOn;
uniform vec3  u_PointLightPos[6];
uniform vec3  u_PointLightColor[6];
uniform float u_PointConstant;
uniform float u_PointLinear;
uniform float u_PointQuadratic;

uniform bool  u_SpotLightOn;
uniform vec3  u_SpotLightPos;
uniform vec3  u_SpotLightDir;
uniform vec3  u_SpotLightColor;
uniform float u_SpotCutoff;
uniform float u_SpotOuterCutoff;
uniform float u_SpotConstant;
uniform float u_SpotLinear;
uniform float u_SpotQuadratic;

float DeskLampSurfaceMask() {
    const vec2 deskCenter = vec2(6.45, 5.95);
    const vec2 deskRadius = vec2(2.2, 1.2);
    vec2 local = (v_FragPos.xz - deskCenter) / deskRadius;
    float radial = 1.0 - dot(local, local);
    float topMask = smoothstep(0.82, 0.96, v_FragPos.y);
    return max(radial, 0.0) * topMask;
}

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

vec3 CalcPointLight(int i, vec3 norm, vec3 viewDir, vec3 baseColor) {
    vec3  lightDir    = normalize(u_PointLightPos[i] - v_FragPos);
    float distance    = length(u_PointLightPos[i] - v_FragPos);
    float attenuation = 1.0 / (u_PointConstant + u_PointLinear * distance + u_PointQuadratic * distance * distance);
    float diff = u_DiffuseOn ? max(dot(norm, lightDir), 0.0) : 0.0;
    vec3 diffuse  = diff * u_PointLightColor[i] * baseColor * attenuation;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = u_SpecularOn ? pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess) : 0.0;
    vec3 specular = spec * u_PointLightColor[i] * attenuation * u_SpecularStrength;
    return diffuse + specular;
}

vec3 CalcSpotLight(vec3 norm, vec3 viewDir, vec3 baseColor) {
    if (!u_SpotLightOn) return vec3(0.0);
    vec3  lightDir    = normalize(u_SpotLightPos - v_FragPos);
    float dist        = length(u_SpotLightPos - v_FragPos);
    float attenuation = 1.0 / (u_SpotConstant + u_SpotLinear * dist + u_SpotQuadratic * dist * dist);
    float theta       = dot(lightDir, normalize(-u_SpotLightDir));
    float epsilon     = u_SpotCutoff - u_SpotOuterCutoff;
    float intensity   = clamp((theta - u_SpotOuterCutoff) / epsilon, 0.0, 1.0) * DeskLampSurfaceMask();
    float diff        = u_DiffuseOn  ? max(dot(norm, lightDir), 0.0) : 0.0;
    vec3  diffuse     = diff * u_SpotLightColor * baseColor * attenuation * intensity;
    vec3  reflectDir  = reflect(-lightDir, norm);
    float spec        = u_SpecularOn ? pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess) : 0.0;
    vec3  specular    = spec * u_SpotLightColor * attenuation * u_SpecularStrength * intensity;
    return diffuse + specular;
}

void main() {
    vec2 uv = v_TexCoord * vec2(u_TileX, u_TileY);
    vec4 texSample = texture(u_Texture, uv);
    // baseColor: per-fragment blend of texture and surface color
    vec3 baseColor = mix(texSample.rgb, u_SurfaceColor, u_BlendFactor);

    if (!u_LightsOn) { FragColor = vec4(baseColor * 0.35, 1.0); return; }

    vec3 norm    = normalize(v_Normal);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    vec3 result = u_AmbientOn ? (u_AmbientStrength * baseColor) : vec3(0.0);
    result += CalcDirLight(norm, viewDir, baseColor);
    if (u_PointLightsOn) {
        result += CalcPointLight(0, norm, viewDir, baseColor);
        result += CalcPointLight(1, norm, viewDir, baseColor);
        result += CalcPointLight(2, norm, viewDir, baseColor);
        result += CalcPointLight(3, norm, viewDir, baseColor);
        result += CalcPointLight(4, norm, viewDir, baseColor);
        result += CalcPointLight(5, norm, viewDir, baseColor);
    }
    result += CalcSpotLight(norm, viewDir, baseColor);
    FragColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}
