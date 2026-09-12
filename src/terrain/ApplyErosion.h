#pragma once

#include <random>
#include <vector>

#include "TerrainPipelineStage.h"

class HydraulicErosion : public TerrainPipelineStage
{
public:
    HydraulicErosion() = default;
    ~HydraulicErosion() = default;

    void apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings) override;

private:
    struct HeightAndGradient
    {
        float height;
        float gradientX;
        float gradientY;
    };

    void initializeBrush(int mapSize, int radius);
    HeightAndGradient calculateHeightAndGradient(const std::vector<float>& map, int mapSize, float posX, float posY);

    mutable std::vector<std::vector<int>>   m_brushIndices;
    mutable std::vector<std::vector<float>> m_brushWeights;
    mutable std::mt19937 m_prng;
    mutable int m_brushMapSize = -1;
    mutable int m_brushRadius  = -1;
};