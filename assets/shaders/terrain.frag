#version 450 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 FragColor;

struct Material
{
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};
uniform Material uMaterial;

uniform sampler2D uSandTexture;
uniform sampler2D uGrassTexture;
uniform sampler2D uRockTexture;
uniform sampler2D uSnowTexture;
uniform float uSandHeight;
uniform float uGrassHeight;
uniform float uRockSlope;
uniform float uSnowHeight;
uniform float uSnowBlendWidth;
uniform float uSandMetallic;
uniform float uGrassMetallic;
uniform float uRockMetallic;
uniform float uSnowMetallic;
uniform float uSandRoughness;
uniform float uGrassRoughness;
uniform float uRockRoughness;
uniform float uSnowRoughness;

#define MAX_LIGHTS 4
uniform vec3 uLightPositions[MAX_LIGHTS];
uniform vec3 uLightDirections[MAX_LIGHTS];
uniform vec3 uLightColors[MAX_LIGHTS];
uniform bool uLightIsDirectional[MAX_LIGHTS];
uniform int uLightCount;

uniform vec3 uCameraPos;
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBRDFLUT;

const float MAX_REFLECTION_LOD = 4.0;
const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0000001);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, 0.0000001);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 baseColor = uMaterial.albedo;

    vec3 N = normalize(vNormal);

    float terrainHeight = vWorldPos.y;
    float slope = 1.0 - clamp(dot(N, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);

    vec3 sandTexture = texture(uSandTexture, vTexCoords).rgb;
    vec3 grassTexture = texture(uGrassTexture, vTexCoords).rgb;
    vec3 rockTexture = texture(uRockTexture, vTexCoords).rgb;
    vec3 snowTexture = texture(uSnowTexture, vTexCoords).rgb;

    vec3 terrainAlbedo = mix(sandTexture, grassTexture, smoothstep(uSandHeight, uGrassHeight, terrainHeight));
    terrainAlbedo = mix(terrainAlbedo, rockTexture, smoothstep(uRockSlope - 0.15, uRockSlope + 0.15, slope));
    terrainAlbedo = mix(terrainAlbedo, snowTexture, smoothstep(uSnowHeight - uSnowBlendWidth, uSnowHeight + uSnowBlendWidth, terrainHeight));

    float grassBlend = smoothstep(uSandHeight, uGrassHeight, terrainHeight);
    float rockBlend = smoothstep(uRockSlope - 0.15, uRockSlope + 0.15, slope);
    float snowBlend = smoothstep(uSnowHeight - uSnowBlendWidth, uSnowHeight + uSnowBlendWidth, terrainHeight);

    float terrainMetallic = mix(uSandMetallic, uGrassMetallic, grassBlend);
    terrainMetallic = mix(terrainMetallic, uRockMetallic, rockBlend);
    terrainMetallic = mix(terrainMetallic, uSnowMetallic, snowBlend);

    float terrainRoughness = mix(uSandRoughness, uGrassRoughness, grassBlend);
    terrainRoughness = mix(terrainRoughness, uRockRoughness, rockBlend);
    terrainRoughness = mix(terrainRoughness, uSnowRoughness, snowBlend);

    baseColor *= terrainAlbedo;
    vec3 V = normalize(uCameraPos - vWorldPos);

    vec3 F0 = mix(vec3(0.04), baseColor, terrainMetallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < uLightCount; ++i)
    {
        vec3 L;
        float attenuation = 1.0;

        if (uLightIsDirectional[i])
        {
            L = normalize(uLightDirections[i]);
        }
        else
        {
            vec3 lightToFragment = uLightPositions[i] - vWorldPos;
            L = normalize(lightToFragment);
            attenuation = 1.0 / max(dot(lightToFragment, lightToFragment), 0.000001);
        }

        vec3 H = normalize(V + L);
        vec3 radiance = uLightColors[i] * attenuation;

        float NDF = distributionGGX(N, H, terrainRoughness);
        float G = geometrySmith(N, V, L, terrainRoughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.0000001);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - terrainMetallic);

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, terrainRoughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - terrainMetallic);

    vec3 irradiance = texture(uIrradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * baseColor;

    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = textureLod(uPrefilterMap, R, terrainRoughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF = texture(uBRDFLUT, vec2(max(dot(N, V), 0.0), terrainRoughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuseIBL + specularIBL) * uMaterial.ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
