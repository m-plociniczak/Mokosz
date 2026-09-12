#include "ApplyIslandMask.h"

#include <glm/gtc/noise.hpp>

void ApplyIslandMask::apply(TerrainHeightField& heightField,
                            const NoiseGenerator::Settings& settings)
{
    for (int z = 0; z < heightField.pointsPerAxis; ++z)
    {
        for (int x = 0; x < heightField.pointsPerAxis; ++x)
        {
            const float worldX = heightField.origin.x + x * heightField.cellSize;
            const float worldZ = heightField.origin.y + z * heightField.cellSize;
            const float distanceFromCenter = glm::length(glm::vec2(worldX, worldZ));

            float islandMask = 1.0f;
            if (settings.islandRadius > 0.0f)
            {
                islandMask = 1.0f - glm::smoothstep(
                    settings.islandRadius - settings.edgeFalloff,
                    settings.islandRadius,
                    distanceFromCenter);
            }

            const int  index = z * heightField.pointsPerAxis + x;
            const float edgeDrop = glm::mix(heightField.heights[static_cast<std::size_t>(index)], heightField.minHeight, 1.0f - islandMask);
            
            heightField.heights[static_cast<std::size_t>(index)] = edgeDrop;
            heightField.maxHeight = std::max(heightField.maxHeight, heightField.heights[static_cast<std::size_t>(index)]);
            heightField.minHeight = std::min(heightField.minHeight, heightField.heights[static_cast<std::size_t>(index)]);
        }
    }
}
