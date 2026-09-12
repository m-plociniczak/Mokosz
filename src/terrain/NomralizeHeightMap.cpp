#include <iostream>

#include "NomralizeHeightMap.hpp"

void NomralizeHeightMap::apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings)
{
    float minHeight = heightField.minHeight;
    float maxHeight = heightField.maxHeight;

    for (int z = 0; z < heightField.pointsPerAxis; ++z)
    {
        for (int x = 0; x < heightField.pointsPerAxis; ++x)
        {
            int index = z * heightField.pointsPerAxis + x;
            float normalizedHeight = (heightField.heights[static_cast<std::size_t>(index)] - minHeight) / (maxHeight - minHeight);
            heightField.heights[static_cast<std::size_t>(index)] = normalizedHeight;

            if(normalizedHeight > 1.f || normalizedHeight < 0.f)
            {
                std::cout << "Normalized height out of bounds at (" << x << ", " << z << "): " << normalizedHeight << std::endl;
            }
        }
    }

    std::cout << "Height map normalized. New min height: " << 0.0f << ", New max height: " << 1.0f << std::endl;
}