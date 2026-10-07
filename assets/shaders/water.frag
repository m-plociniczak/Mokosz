#version 450 core

in vec3 vWorldPosition;
out vec4 FragColor;

struct Material
{
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

uniform Material uMaterial;
uniform vec3 uCameraPos;
uniform float uTime;

void main()
{
    vec2 position = vWorldPosition.xz;
    vec2 directionA = normalize(vec2(0.88, 0.47));
    vec2 directionB = normalize(vec2(0.94, 0.34));
    vec2 directionC = normalize(vec2(0.81, 0.59));

    float phaseA = dot(position, directionA) * 0.16 - uTime * 0.7;
    float phaseB = dot(position, directionB) * 0.25 - uTime * 0.95;
    float phaseC = dot(position, directionC) * 0.09 - uTime * 0.45;
    float waveA = sin(phaseA);
    float waveB = sin(phaseB);
    float waveC = sin(phaseC);
    float waves = waveA * 0.58 + waveB * 0.27 + waveC * 0.15;

    vec2 waveSlope =
        directionA * (cos(phaseA) * 0.16 * 0.58) +
        directionB * (cos(phaseB) * 0.25 * 0.27) +
        directionC * (cos(phaseC) * 0.09 * 0.15);
    vec3 normal = normalize(vec3(-waveSlope.x * 4.0, 1.0, -waveSlope.y * 4.0));
    vec3 viewDirection = normalize(uCameraPos - vWorldPosition);
    vec3 lightDirection = normalize(vec3(-0.4, 0.85, 0.3));
    vec3 halfDirection = normalize(lightDirection + viewDirection);

    float fresnel = pow(1.0 - max(dot(normal, viewDirection), 0.0), 4.0);
    float specular = pow(max(dot(normal, halfDirection), 0.0), 48.0);
    float crest = smoothstep(0.35, 0.9, waves);

    vec3 deepBlue = uMaterial.albedo * 0.62;
    vec3 shallowBlue = vec3(0.025, 0.34, 0.52);
    vec3 color = mix(deepBlue, shallowBlue, 0.2 + crest * 0.28 + fresnel * 0.16);
    color += vec3(0.28, 0.68, 0.82) * (specular * 0.16 + crest * 0.06);

    FragColor = vec4(color, 1.0);
}
