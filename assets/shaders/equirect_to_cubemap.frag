#version 450 core

in vec3 vLocalPos;
out vec4 FragColor;

uniform sampler2D uEquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // 1/(2*PI), 1/PI

// Converts a direction vector into equirectangular (lat/long) UV coordinates.
vec2 sampleSphericalMap(vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = sampleSphericalMap(normalize(vLocalPos));
    vec3 color = texture(uEquirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
