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
uniform bool uHasDiffuseTexture;
uniform sampler2D uDiffuseTexture;

#define MAX_LIGHTS 4
uniform vec3 uLightPositions[MAX_LIGHTS];
uniform vec3 uLightDirections[MAX_LIGHTS];
uniform vec3 uLightColors[MAX_LIGHTS]; // radiance at the source, not pre-attenuated
uniform bool uLightIsDirectional[MAX_LIGHTS];
uniform int uLightCount;

uniform vec3 uCameraPos;
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBRDFLUT;

const float MAX_REFLECTION_LOD = 4.0; // mipLevels - 1, must match IBLGenerator::prefilterEnvironment's mipLevels

const float PI = 3.14159265359;

// GGX / Trowbridge-Reitz normal distribution function.
// Estimates how many microfacets are aligned with the halfway vector H --
// this is what produces the bright, tight highlight for low roughness and
// the soft, spread-out highlight for high roughness.
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

// Schlick-GGX approximation of geometric self-shadowing/masking for a
// single direction (used for both view and light direction via Smith's method).
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // direct lighting remapping, per Karis/Epic

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, 0.0000001);
}

// Smith's method: multiplies the geometry term for the view direction and
// the light direction, accounting for both shadowing and masking.
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel-Schlick approximation: how much light reflects vs refracts at
// this angle. F0 is the base reflectivity at a direct (0 degree) angle --
// ~0.04 for all dielectrics, or the albedo color itself for metals.
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Roughness-aware Fresnel variant used only for the ambient/IBL term.
// Plain fresnelSchlick assumes a perfectly smooth surface; at grazing
// angles this over-brightens rough materials in ambient light, so this
// version clamps the effective F0 range based on roughness instead.
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 baseColor = uMaterial.albedo;
    if (uHasDiffuseTexture)
    {
        baseColor *= texture(uDiffuseTexture, vTexCoords).rgb;
    }

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    // Dielectrics (non-metals) reflect ~4% of light at normal incidence
    // regardless of color; metals tint their reflection with their albedo
    // and have no diffuse term at all. This single mix() encodes both cases.
    vec3 F0 = mix(vec3(0.04), baseColor, uMaterial.metallic);

    vec3 Lo = vec3(0.0); // outgoing radiance, accumulated per light

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

        // Cook-Torrance specular term: D * G * F / (4 * NdotV * NdotL)
        float NDF = distributionGGX(N, H, uMaterial.roughness);
        float G = geometrySmith(N, V, L, uMaterial.roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.0000001);

        // kS = F (energy reflected as specular); kD is what's left for
        // diffuse. Metals absorb all remaining light (no diffuse term).
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - uMaterial.metallic);

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    // Diffuse IBL: replaces the old flat vec3(0.03) placeholder with real
    // ambient light sampled from the environment.
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, uMaterial.roughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - uMaterial.metallic);

    vec3 irradiance = texture(uIrradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * baseColor;

    // Specular IBL: sample the prefiltered environment at the mip matching
    // this roughness, then apply the split-sum BRDF LUT (scale + bias).
    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = textureLod(uPrefilterMap, R, uMaterial.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF = texture(uBRDFLUT, vec2(max(dot(N, V), 0.0), uMaterial.roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuseIBL + specularIBL) * uMaterial.ao;

    vec3 color = ambient + Lo;

    // Reinhard tone mapping + gamma correction. This will move to a
    // dedicated post-processing pass later; inlined here for now since
    // there is no framebuffer stage yet.
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
