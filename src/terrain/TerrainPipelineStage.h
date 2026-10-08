#pragma once

#include <glm/glm.hpp>

#include "NoiseGenerator.h"
#include "TerrainHeightField.h"

class TerrainPipelineStage
{
public:
    virtual ~TerrainPipelineStage() = default;
    virtual void apply(TerrainHeightField& heightField) = 0;
};
