#include "ApplyAmplitude.hpp"
#include <iostream>

void ApplyAmplitude::apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings)
{
    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::lowest();

    for (int z = 0; z < heightField.pointsPerAxis; ++z)
    {
        for (int x = 0; x < heightField.pointsPerAxis; ++x)
        {
            int index = z * heightField.pointsPerAxis + x;
            heightField.heights[static_cast<std::size_t>(index)] *= settings.amplitude;

            minHeight = std::min(minHeight, heightField.heights[static_cast<std::size_t>(index)]);
            maxHeight = std::max(maxHeight, heightField.heights[static_cast<std::size_t>(index)]);
        }
    }

    std::cout << "Amplitude: " << settings.amplitude << " applied to height map." << std::endl;
    std::cout << "New min height: " << minHeight << ", New max height: " << maxHeight << std::endl;
}