#include "NoiseGenerator.h"

#include <glm/gtc/noise.hpp>
#include <iostream>


glm::vec3 NoiseGenerator::getGradientAtPoint(float x, float z) 
{
    float delta = 0.01f;


    m_settings.noiseType = Settings::NoiseType::PlainPerlin;

    float heightL = getHeight(x - delta, z);
    float heightR = getHeight(x + delta, z);
    float heightD = getHeight(x, z - delta);
    float heightU = getHeight(x, z + delta);

    m_settings.noiseType = Settings::NoiseType::GradientTrickPerlin;    
    return glm::vec3(heightR - heightL, 2.0f * delta, heightU - heightD);
}





NoiseGenerator::NoiseGenerator(const Settings& settings)
    : m_settings(settings){}

float NoiseGenerator::getHeight(float worldX, float worldZ)
{
    if(m_settings.noiseType == Settings::NoiseType::PlainPerlin)                return perlinNoise(worldX, worldZ);
    if(m_settings.noiseType == Settings::NoiseType::GradientTrickPerlin)        return gradientTrickPerlinNoise(worldX, worldZ);
    if(m_settings.noiseType == Settings::NoiseType::DomainWarpedPerlin)         return domainWarpedPerlinNoise(worldX, worldZ);
    
    std::cerr << "Unknown noise type selected. " << std::endl;
    return 0.0f;
  
}


float NoiseGenerator::domainWarpedPerlinNoise(float worldX, float worldZ)
{
    float total = 0.0f;
    float frequency = 1.0f / m_settings.scale;
    float amplitude = 1.0f;

    for (int octave = 0; octave < m_settings.octaves; ++octave)
    {
        glm::vec2 samplePoint(
            worldX * frequency + m_settings.seedOffset.x,
            worldZ * frequency + m_settings.seedOffset.y);

        glm::vec2 warpOffset(
            glm::perlin(samplePoint + glm::vec2(5.2f, 1.3f)) * 10.0f,
            glm::perlin(samplePoint + glm::vec2(8.3f, 2.8f)) * 10.0f);

        total += glm::perlin(samplePoint + warpOffset) * amplitude;

        amplitude *= m_settings.persistence;
        frequency *= m_settings.lacunarity;
    }
    return total;
}

float NoiseGenerator::gradientTrickPerlinNoise(float worldX, float worldZ)
{
    float total = 0.0f;
    float frequency = 1.0f / m_settings.scale;
    float amplitude = 1.0f;
    float delta = 0.01f;
    glm::vec2 prevGradient(0.0f, 0.0f);

    for (int octave = 0; octave < m_settings.octaves; ++octave)
    {
        glm::vec2 samplePoint(
            worldX * frequency + m_settings.seedOffset.x,
            worldZ * frequency + m_settings.seedOffset.y);
        
   
        float curentHeight  = glm::perlin(samplePoint);
        prevGradient       += glm::vec2(glm::perlin(samplePoint + glm::vec2(delta, 0.0f)) - curentHeight / delta, 
                                        glm::perlin(samplePoint + glm::vec2(0.0f, delta)) - curentHeight / delta) * amplitude;

        float dampingFactor = 1.0f / (1 + glm::length(prevGradient) * m_settings.gradientTrickStrength);


        total += glm::perlin(samplePoint) * amplitude * dampingFactor;

        amplitude *= m_settings.persistence;
        frequency *= m_settings.lacunarity;
    }
    return total;
}


float NoiseGenerator::perlinNoise(float worldX, float worldZ)
{
    float total = 0.0f;
    float frequency = 1.0f / m_settings.scale;
    float amplitude = 1.0f;

    for (int octave = 0; octave < m_settings.octaves; ++octave)
    {
        glm::vec2 samplePoint(
            worldX * frequency + m_settings.seedOffset.x,
            worldZ * frequency + m_settings.seedOffset.y);

        total += glm::perlin(samplePoint) * amplitude;

        amplitude *= m_settings.persistence;
        frequency *= m_settings.lacunarity;
    }

    return total;
}


