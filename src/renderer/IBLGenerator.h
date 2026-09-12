#pragma once

#include <memory>

#include "HDRTexture.h"
#include "Cubemap.h"
#include "Mesh.h"
#include "Texture.h"

// Static helpers that turn an equirectangular HDR panorama into IBL data.
// This first stage only covers the environment cubemap conversion;
// irradiance convolution and specular prefiltering are added in later
// stages, following the same "render into each cubemap face" pattern.
class IBLGenerator
{
public:
    // Renders the given equirectangular panorama into a new cubemap by
    // drawing a unit cube 6 times (once per face) from the origin, with a
    // 90-degree FOV camera pointed along each axis. faceSize=512 is a
    // reasonable default: large enough to look sharp as a skybox, small
    // enough to convolve quickly in the next IBL stages.
    static std::unique_ptr<Cubemap> equirectangularToCubemap(const HDRTexture& hdrEquirect, int faceSize = 512);

    // Convolves an environment cubemap into an irradiance map: for every
    // direction, integrates incoming light over the hemisphere above it.
    // The result is very low-frequency (diffuse light has no sharp
    // detail), so a small faceSize like 32 is enough and keeps the bake
    // fast despite the expensive per-pixel hemisphere integral.
    static std::unique_ptr<Cubemap> convolveIrradiance(const Cubemap& environmentCubemap, int faceSize = 32);

    // Renders the environment into a mipmapped cubemap where each mip
    // level stores the environment pre-blurred for a specific roughness
    // (mip 0 = roughness 0 / sharp, last mip = roughness 1 / fully rough).
    // Sampling with textureLod(prefilterMap, R, roughness * (mipCount-1))
    // in the shader then gives a roughness-appropriate reflection.
    static std::unique_ptr<Cubemap> prefilterEnvironment(const Cubemap& environmentCubemap, int baseFaceSize = 128, int mipLevels = 5);

    // Precomputes the BRDF integral (scale + bias for F0) as a function of
    // (NdotV, roughness), independent of any particular environment --
    // this can be generated once and reused across every material/scene.
    static Texture generateBRDFLUT(int size = 512);
};
