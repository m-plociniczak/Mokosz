#pragma once

#include <glm/glm.hpp>

#include "TerrainPipelineStage.h"
#include "NoiseGenerator.h" 

class ApplyIslandMask : public TerrainPipelineStage
{
    public:
        ApplyIslandMask(float radius = 90.0f, float falloff = 18.0f, const glm::vec2& seedOffset = glm::vec2(0.0f, 0.0f)) 
            : m_islandRadius(radius), m_edgeFalloff(falloff), m_seedOffset(seedOffset) {}
        
        void apply(TerrainHeightField& heightField) override;

        float getIslandRadius() const { return m_islandRadius; }
        void setIslandRadius(float radius) { m_islandRadius = radius; }

        float getEdgeFalloff() const { return m_edgeFalloff; }
        void setEdgeFalloff(float falloff) { m_edgeFalloff = falloff; }

        glm::vec2 getSeedOffset() const { return m_seedOffset; }
        void setSeedOffset(const glm::vec2& seedOffset) { m_seedOffset = seedOffset; }

    private:
        float m_islandRadius;
        float m_edgeFalloff;
        glm::vec2 m_seedOffset = glm::vec2(0.0f, 0.0f);
};
