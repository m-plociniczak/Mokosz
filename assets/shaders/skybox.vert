#version 450 core

layout(location = 0) in vec3 aPosition;

out vec3 vLocalPos;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vLocalPos = aPosition;

    // Strip translation from the view matrix so the skybox is always
    // centered on the camera regardless of where it's standing.
    mat4 rotationOnlyView = mat4(mat3(uView));
    vec4 clipPos = uProjection * rotationOnlyView * vec4(aPosition, 1.0);

    // Force depth to the far plane (w == z after the perspective divide)
    // so the skybox renders behind every other object without needing to
    // disable depth testing.
    gl_Position = clipPos.xyww;
}
