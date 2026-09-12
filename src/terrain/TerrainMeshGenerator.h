#pragma once

#include "NoiseGenerator.h"
#include "TerrainPipeline.h"
#include "../renderer/Mesh.h"


class TerrainMeshGenerator
{
public:
    static Mesh generatePatch(const NoiseGenerator& noise,
                               const glm::vec2& origin,
                               float size,
                               int resolution,
                               std::vector<std::shared_ptr<Texture>> textures = {});

private:
    static glm::vec3 estimateNormal(const NoiseGenerator& noise, float worldX, float worldZ, float epsilon);
};
