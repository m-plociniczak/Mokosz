#version 450 core

layout(location = 0) in vec3 aPosition;

out vec3 vLocalPos;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    // The unit cube's local position IS the direction from its center --
    // that's exactly what we need to sample the equirectangular panorama.
    vLocalPos = aPosition;
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
}
