#pragma once

#include <glm/glm.hpp>


class NoiseGenerator
{
public:
    struct Settings
    {
        int octaves = 5;
        float persistence = 0.52f;                      // amplitude multiplier per octave (< 1 = each octave contributes less)
        float lacunarity = 2.0f;                        // frequency multiplier per octave (> 1 = each octave adds finer detail)
        float scale = 140.0f;                           // world-units per base noise cycle; larger = broader, gentler hills
        float amplitude = 18.0f;                        // final height range in world units (output is roughly -amplitude..amplitude)
        float islandRadius = 90.0f;                     // radius from the terrain center where the island remains fully visible
        float edgeFalloff = 18.0f;                      // width of the fade-out zone toward the island edges
        float edgeDrop = 8.0f;                          // extra downward displacement applied near the island perimeter
        float gradientTrickStrength = 0.6f;             // strength of the gradient trick for Perlin noise (0 - > 1)


        enum NoiseType
        {
            PlainPerlin,
            GradientTrickPerlin, 
        } 
        
        noiseType = NoiseType::PlainPerlin;
        glm::vec2 seedOffset = glm::vec2(0.0f);

        bool enableErosion = false;

        int iterations = 2000;
        int dropletLifetime = 30;
        float brushRadius = 2.5f;
        float erosionStrength = 0.5f;
        float sedimentCapacity = 0.8f;
        float depositionSpeed = 0.12f;
        float evaporationRate = 0.03f;

        float inertia = .05f;                       // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
        float sedimentCapacityFactor = 4;           // Multiplier for how much sediment a droplet can carry
        float minSedimentCapacity = .01f;           // Used to prevent carry capacity getting too close to zero on flatter terrain
        float erodeSpeed = .3f;                     // Speed of erosion (0,1)
        float gravity = 4;                          // Downward acceleration of a droplet
        float maxDropletLifetime = 30;              // The number of steps a droplet can take before it dies
        float initialWaterVolume = 1;               // The starting water volume of a droplet
        float initialSpeed = 1;                     // The starting speed of a droplet
    };

    explicit NoiseGenerator(const Settings& settings = Settings());

    float getHeight(float worldX, float worldZ);

    void setSettings(const Settings& settings) { m_settings = settings; }
    const Settings& settings() const { return m_settings; }

private:
    Settings m_settings;

    float perlinNoise(float worldX, float worldZ);               
    float gradientTrickPerlinNoise(float worldX, float worldZ);
    glm::vec3 getGradientAtPoint(float x, float z);
};
