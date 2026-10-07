#include "ApplyIslandMask.h"

#include <glm/gtc/noise.hpp>
#include <glm/glm.hpp>

void ApplyIslandMask::apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings)
{
    for (int z = 0; z < heightField.pointsPerAxis; ++z)
    {
        for (int x = 0; x < heightField.pointsPerAxis; ++x)
        {
            const float worldX                  =  heightField.origin.x + x * heightField.cellSize;
            const float worldZ                  =  heightField.origin.y + z * heightField.cellSize;
            const float angle                   =  glm::atan(worldZ, worldX);                                // -pi to pi
            const float noiseAtDirection        =  glm::perlin(glm::vec2(std::cos(angle) + settings.seedOffset.x, std::sin(angle) + settings.seedOffset.y)) * settings.islandRadius * 0.50f;
            const float distanceFromCenter      =  glm::length(glm::vec2(worldX, worldZ)) + noiseAtDirection;
            
            if(distanceFromCenter > settings.islandRadius)
            {
                const float distanceBeyondRadius = distanceFromCenter - settings.islandRadius;
                const float fadeFactor           = glm::clamp(distanceBeyondRadius / settings.edgeFalloff, 0.0f, 1.0f);

                const int index = z * heightField.pointsPerAxis + x;
                heightField.heights[index] *= 1.0f - fadeFactor;
                
            }

        }
    }
}
