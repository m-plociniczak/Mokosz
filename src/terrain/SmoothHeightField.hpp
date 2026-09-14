#pragma once

#include "TerrainPipelineStage.h"

class SmoothHeightField : public TerrainPipelineStage
{
    
    public:
        explicit SmoothHeightField(int kernelRadius = 3);
        ~SmoothHeightField() = default;

        void apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings) override;
    
    private:
        int m_kernelRadius;
};
