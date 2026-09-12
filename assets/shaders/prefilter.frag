#version 450 core

in vec3 vLocalPos;
out vec4 FragColor;

uniform samplerCube uEnvironmentMap;
uniform float uRoughness;

const float PI = 3.14159265359;

// Same GGX distribution as pbr.frag -- duplicated here rather than shared
// via an #include, since this project doesn't have a shader preprocessor
// set up yet for cross-file includes.
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

// Van der Corput sequence + Hammersley set: a fast way to generate a
// well-distributed, low-discrepancy 2D sample sequence without random
// number generation (deterministic and GPU-friendly).
float radicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radicalInverseVdC(i));
}

// Importance-samples a half-vector H biased toward the GGX distribution
// for the given roughness, instead of sampling directions uniformly --
// this is what makes convergence with a limited sample count feasible.
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

void main()
{
    vec3 N = normalize(vLocalPos);
    // Approximation used throughout this split-sum technique: view
    // direction assumed equal to the normal (no way to know the actual
    // view direction ahead of time, since this bakes into a cubemap used
    // from any angle later).
    vec3 V = N;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, uRoughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // Weighting by NdotL (rather than a plain average) matches
            // how these samples would contribute in the real reflectance
            // integral -- grazing samples contribute less.
            prefilteredColor += texture(uEnvironmentMap, L).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / max(totalWeight, 0.0001);

    FragColor = vec4(prefilteredColor, 1.0);
}
