#pragma once

#include "TerrainPipelineStage.h"

class SmoothHeightField : public TerrainPipelineStage
{
    
    public:
        explicit SmoothHeightField(int kernelRadius = 3);
        ~SmoothHeightField() = default;

        void apply(TerrainHeightField& heightField) override;

        int getKernelRadius() const { return m_kernelRadius; }
        void setKernelRadius(int kernelRadius) { m_kernelRadius = kernelRadius; }
    
    private:
        int m_kernelRadius;
};
