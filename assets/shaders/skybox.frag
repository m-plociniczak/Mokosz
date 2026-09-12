#version 450 core

in vec3 vLocalPos;
out vec4 FragColor;

uniform samplerCube uEnvironmentMap;

void main()
{
    vec3 color = texture(uEnvironmentMap, normalize(vLocalPos)).rgb;

    // Same tone mapping as pbr.frag -- keeps the skybox visually consistent
    // with lit objects instead of looking blown-out next to them.
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
