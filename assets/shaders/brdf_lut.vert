#version 450 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords;

out vec2 vTexCoords;

void main()
{
    vTexCoords = aTexCoords;
    // Already in NDC (-1..1) -- this quad is drawn directly to a
    // full-viewport target, no camera/MVP transform needed.
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
