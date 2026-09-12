#version 450 core

in vec3 vLocalPos;
out vec4 FragColor;

uniform samplerCube uEnvironmentMap;

const float PI = 3.14159265359;

void main()
{
    // The direction from the cube's center IS the surface normal we're
    // computing irradiance for -- same trick as equirect_to_cubemap.frag,
    // just reused here for a different purpose.
    vec3 N = normalize(vLocalPos);

    vec3 irradiance = vec3(0.0);

    // Build an arbitrary tangent basis around N. The exact orientation
    // doesn't matter -- we're about to integrate over the whole
    // hemisphere symmetrically, so no particular "up" direction is special.
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    // Riemann sum over the hemisphere in spherical coordinates. Smaller
    // sampleDelta = smoother result but more samples (and slower bake);
    // 0.025 radians is the common default -- fine since this only runs
    // once at startup, not per frame.
    float sampleDelta = 0.025;
    int sampleCount = 0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Sample direction in the local tangent space (theta measured
            // from N, phi rotating around it)...
            vec3 tangentSample = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta));

            // ...then rotated into world space using the basis built above.
            vec3 sampleDir = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            // cos(theta): Lambert's cosine law -- light arriving at a
            // grazing angle contributes less.
            // sin(theta): accounts for the shrinking ring area near the
            // pole in spherical coordinates (solid angle correction).
            irradiance += texture(uEnvironmentMap, sampleDir).rgb * cos(theta) * sin(theta);
            ++sampleCount;
        }
    }

    // Normalizing by sampleCount averages the Riemann sum; the extra PI
    // factor converts the result into proper irradiance (matches the
    // Lambertian BRDF's albedo/PI term used later in pbr.frag).
    irradiance = PI * irradiance / float(sampleCount);

    FragColor = vec4(irradiance, 1.0);
}
