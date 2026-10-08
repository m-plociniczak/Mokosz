#pragma once

#include <random>
#include <vector>
#include <glm/glm.hpp>

#include "TerrainPipelineStage.h"

class HydraulicErosion : public TerrainPipelineStage
{
public:
    HydraulicErosion() = default;
    ~HydraulicErosion() = default;

    bool getEnableErosion() const { return m_enableErosion; }
    void setEnableErosion(bool value) { m_enableErosion = value; }

    int getIterations() const { return m_iterations; }
    void setIterations(int value) { m_iterations = value; }

    int getDropletLifetime() const { return m_dropletLifetime; }
    void setDropletLifetime(int value) { m_dropletLifetime = value; }

    float getBrushRadius() const { return m_brushRadius; }
    void setBrushRadius(float value) { m_brushRadius = value; }

    float getErosionStrength() const { return m_erosionStrength; }
    void setErosionStrength(float value) { m_erosionStrength = value; }

    float getSedimentCapacity() const { return m_sedimentCapacity; }
    void setSedimentCapacity(float value) { m_sedimentCapacity = value; }

    float getDepositionSpeed() const { return m_depositionSpeed; }
    void setDepositionSpeed(float value) { m_depositionSpeed = value; }

    float getEvaporationRate() const { return m_evaporationRate; }
    void setEvaporationRate(float value) { m_evaporationRate = value; }

    float getInertia() const { return m_inertia; }
    void setInertia(float value) { m_inertia = value; }

    float getSedimentCapacityFactor() const { return m_sedimentCapacityFactor; }
    void setSedimentCapacityFactor(float value) { m_sedimentCapacityFactor = value; }

    float getMinSedimentCapacity() const { return m_minSedimentCapacity; }
    void setMinSedimentCapacity(float value) { m_minSedimentCapacity = value; }

    float getErodeSpeed() const { return m_erodeSpeed; }
    void setErodeSpeed(float value) { m_erodeSpeed = value; }

    float getGravity() const { return m_gravity; }
    void setGravity(float value) { m_gravity = value; }

    float getMaxDropletLifetime() const { return m_maxDropletLifetime; }
    void setMaxDropletLifetime(float value) { m_maxDropletLifetime = value; }

    float getInitialWaterVolume() const { return m_initialWaterVolume; }
    void setInitialWaterVolume(float value) { m_initialWaterVolume = value; }

    float getInitialSpeed() const { return m_initialSpeed; }
    void setInitialSpeed(float value) { m_initialSpeed = value; }

    glm::vec2 getSeedOffset() const { return m_seedOffset; }
    void setSeedOffset(const glm::vec2& value) { m_seedOffset = value; }

    void apply(TerrainHeightField& heightField) override;

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
    mutable std::mt19937                    m_prng;
    mutable int                             m_brushMapSize = -1;

    // ====================== EROSION PARAMETERS ====================== //
    bool        m_enableErosion             = false;
    int         m_iterations                = 2000;
    int         m_dropletLifetime           = 30;
    int         m_brushRadius               = 3;
    float       m_erosionStrength           = 0.5f;
    float       m_sedimentCapacity          = 0.8f;
    float       m_depositionSpeed           = 0.12f;
    float       m_evaporationRate           = 0.03f;
    float       m_inertia                   = .05f;                       // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
    float       m_sedimentCapacityFactor    = 4;           // Multiplier for how much sediment a droplet can carry
    float       m_minSedimentCapacity       = .01f;           // Used to prevent carry capacity getting too close to zero on flatter terrain
    float       m_erodeSpeed                = .3f;                     // Speed of erosion (0,1)
    float       m_gravity                   = 4;                          // Downward acceleration of a droplet
    float       m_maxDropletLifetime        = 30;              // The number of steps a droplet can take before it dies
    float       m_initialWaterVolume        = 1;               // The starting water volume of a droplet
    float       m_initialSpeed              = 1;                     // The starting speed of a droplet
    glm::vec2   m_seedOffset                = glm::vec2(0.0f);
};