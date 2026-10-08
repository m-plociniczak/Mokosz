#pragma once

#include "TerrainPipelineStage.h"

class CreateNoiseMapObject : public TerrainPipelineStage
{
public:
    CreateNoiseMapObject(const NoiseGenerator& noiseGenerator) : m_noiseGenerator(&noiseGenerator) {}
    void apply(TerrainHeightField& heightField) override;

    const NoiseGenerator& getNoiseGenerator() const { return *m_noiseGenerator; }
    void setNoiseGenerator(const NoiseGenerator& noiseGenerator) { m_noiseGenerator = &noiseGenerator; }

private:
    const NoiseGenerator* m_noiseGenerator;
};
