#pragma once
#include "TerrainPipelineStage.h"

class ApplyAmplitude : public TerrainPipelineStage
{
public:
    ApplyAmplitude(float amplitude = 18.0f)     :    m_amplitude(amplitude) {}      
    void apply(TerrainHeightField& heightField)     override;

    float getAmplitude() const { return m_amplitude; }
    void setAmplitude(float amplitude) { m_amplitude = amplitude; }

private:
    float m_amplitude;
};
