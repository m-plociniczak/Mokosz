#pragma once

#include "TerrainPipelineStage.h"
#include "NoiseGenerator.h" 

class ApplyIslandMask : public TerrainPipelineStage
{
public:
    void apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings) override;
};
