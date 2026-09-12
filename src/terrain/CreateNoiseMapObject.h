#pragma once

#include "TerrainPipelineStage.h"

class CreateNoiseMapObject : public TerrainPipelineStage
{
public:
    void apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings) override;
};
