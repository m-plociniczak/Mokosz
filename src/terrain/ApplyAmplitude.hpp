#pragma once
#include "TerrainPipelineStage.h"

class ApplyAmplitude : public TerrainPipelineStage
{
public:
    void apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings) override;
};
