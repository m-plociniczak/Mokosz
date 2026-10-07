#include "CreateNoiseMapObject.h"
#include <iostream>
#include <vector>

void CreateNoiseMapObject::apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings)
{
    heightField.minHeight = std::numeric_limits<float>::max();
    heightField.maxHeight = std::numeric_limits<float>::lowest();

    for (int z = 0; z < heightField.pointsPerAxis; ++z)
    {
        for (int x = 0; x < heightField.pointsPerAxis; ++x)
        {
            const float worldX = heightField.origin.x + x * heightField.cellSize;
            const float worldZ = heightField.origin.y + z * heightField.cellSize;
            const float height = NoiseGenerator(settings).getHeight(worldX, worldZ);

            heightField.minHeight = std::min(heightField.minHeight, height);
            heightField.maxHeight = std::max(heightField.maxHeight, height);

            heightField.heights[static_cast<std::size_t>(z) * heightField.pointsPerAxis + x] = height;
        }
    }

    std::cout << "Noise map generated. Min height: " << heightField.minHeight << ", Max height: " << heightField.maxHeight << std::endl;
}
