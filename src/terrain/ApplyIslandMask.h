#pragma once

#include "TerrainPipelineStage.h"

class ApplyIslandMask : public TerrainPipelineStage
{
public:
    void apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings) override;
};
