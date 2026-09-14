#include "SmoothHeightField.hpp"

SmoothHeightField::SmoothHeightField(int kernelRadius)
    : m_kernelRadius(kernelRadius)
{}

void SmoothHeightField::apply(TerrainHeightField& heightField, const NoiseGenerator::Settings& settings)
{
        const int mapSize = heightField.pointsPerAxis;
        if (mapSize < 3) return;

        std::vector<float> src = heightField.heights;
        auto& dst = heightField.heights;

        for (int z = 0; z < mapSize; ++z)
        {
            for (int x = 0; x < mapSize; ++x)
            {
                float sum = 0.0f;
                int count = 0;
                for (int dz = -m_kernelRadius; dz <= m_kernelRadius; ++dz)
                {
                    for (int dx = -m_kernelRadius; dx <= m_kernelRadius; ++dx)
                    {
                        const int nx = std::clamp(x + dx, 0, mapSize - 1);
                        const int nz = std::clamp(z + dz, 0, mapSize - 1);
                        sum += src[static_cast<std::size_t>(nz) * mapSize + nx];
                        ++count;
                    }
                }
                dst[static_cast<std::size_t>(z) * mapSize + x] = sum / static_cast<float>(count);
            }
        }
    }