#include "ApplyErosion.h"

#include <algorithm>
#include <cmath>
#include <iostream>

void HydraulicErosion::initializeBrush(int mapSize, int radius)
{
    if (mapSize == m_brushMapSize && radius == m_brushRadius)
        return; 

    m_brushMapSize = mapSize;
    m_brushRadius = radius;

    m_brushIndices.assign(static_cast<std::size_t>(mapSize) * mapSize, {});
    m_brushWeights.assign(static_cast<std::size_t>(mapSize) * mapSize, {});

    std::vector<int> xOffsets(static_cast<std::size_t>(radius) * radius * 4);
    std::vector<int> yOffsets(xOffsets.size());
    std::vector<float> weights(xOffsets.size());

    for (int i = 0; i < mapSize * mapSize; ++i)
    {
        const int centreX = i % mapSize;
        const int centreY = i / mapSize;

        float weightSum = 0.0f;
        int addIndex = 0;

        for (int y = -radius; y <= radius; ++y)
        {
            for (int x = -radius; x <= radius; ++x)
            {
                const float sqrDst = static_cast<float>(x * x + y * y);
                if (sqrDst < static_cast<float>(radius * radius))
                {
                    const int coordX = centreX + x;
                    const int coordY = centreY + y;
                    if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize)
                    {
                        const float weight = 1.0f - std::sqrt(sqrDst) / static_cast<float>(radius);
                        weightSum += weight;
                        weights[addIndex] = weight;
                        xOffsets[addIndex] = x;
                        yOffsets[addIndex] = y;
                        ++addIndex;
                    }
                }
            }
        }

        auto& indices = m_brushIndices[i];
        auto& brushWeights = m_brushWeights[i];
        indices.resize(addIndex);
        brushWeights.resize(addIndex);
        for (int j = 0; j < addIndex; ++j)
        {
            indices[j] = (yOffsets[j] + centreY) * mapSize + (xOffsets[j] + centreX);
            brushWeights[j] = weights[j] / weightSum;
        }
    }
}

HydraulicErosion::HeightAndGradient HydraulicErosion::calculateHeightAndGradient(const std::vector<float>& map, int mapSize, float posX, float posY)
{
    const int coordX = static_cast<int>(posX);
    const int coordY = static_cast<int>(posY);

    const float x = posX - coordX;
    const float y = posY - coordY;

    const int nodeIndexNW = coordY * mapSize + coordX;
    const float heightNW = map[nodeIndexNW];
    const float heightNE = map[nodeIndexNW + 1];
    const float heightSW = map[nodeIndexNW + mapSize];
    const float heightSE = map[nodeIndexNW + mapSize + 1];

    const float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
    const float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

    const float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y)
                        + heightSW * (1 - x) * y       + heightSE * x * y;

    return HeightAndGradient{ height, gradientX, gradientY };
}

void HydraulicErosion::apply(TerrainHeightField& heightField)
{
    if (!m_enableErosion)
        return;

    const int mapSize = heightField.pointsPerAxis;
    if (mapSize < 3)
        return;

    std::vector<float>& heights = heightField.heights;

    const int radius = std::clamp(static_cast<int>(std::lround(m_brushRadius)), 2, 8);
    initializeBrush(mapSize, radius);
    m_prng.seed(m_seedOffset.x + m_seedOffset.y); 

    std::uniform_real_distribution<float> posDist(0.0f, static_cast<float>(mapSize - 1));

    for (int iteration = 0; iteration < m_iterations; ++iteration)
    {
        float posX = posDist(m_prng);
        float posY = posDist(m_prng);
        float dirX = 0.0f;
        float dirY = 0.0f;
        float speed = m_initialSpeed;
        float water = m_initialWaterVolume;
        float sediment = 0.0f;

        for (int lifetime = 0; lifetime < m_maxDropletLifetime; ++lifetime)
        {
            const int nodeX = static_cast<int>(posX);
            const int nodeY = static_cast<int>(posY);
            const int dropletIndex = nodeY * mapSize + nodeX;

            const float cellOffsetX = posX - nodeX;
            const float cellOffsetY = posY - nodeY;

            const HeightAndGradient hg = calculateHeightAndGradient(heights, mapSize, posX, posY);

            dirX = dirX * m_inertia - hg.gradientX * (1.0f - m_inertia);
            dirY = dirY * m_inertia - hg.gradientY * (1.0f - m_inertia);

            const float len = std::sqrt(dirX * dirX + dirY * dirY);
            if (len != 0.0f)
            {
                dirX /= len;
                dirY /= len;
            }

            posX += dirX;
            posY += dirY;

            if ((dirX == 0.0f && dirY == 0.0f) ||
                posX < 0.0f || posX >= mapSize - 1 ||
                posY < 0.0f || posY >= mapSize - 1)
            {
                break;
            }

            const float newHeight = calculateHeightAndGradient(heights, mapSize, posX, posY).height;
            const float deltaHeight = newHeight - hg.height;

            const float sedimentCapacity = std::max(
                -deltaHeight * speed * water * m_sedimentCapacity,
                m_minSedimentCapacity);

            if (sediment > sedimentCapacity || deltaHeight > 0.0f)
            {
                const float amountToDeposit = (deltaHeight > 0.0f)
                    ? std::min(deltaHeight, sediment)
                    : (sediment - sedimentCapacity) * m_depositionSpeed;
                sediment -= amountToDeposit;

                heights[dropletIndex]               += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
                heights[dropletIndex + 1]           += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
                heights[dropletIndex + mapSize]     += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
                heights[dropletIndex + mapSize + 1] += amountToDeposit * cellOffsetX * cellOffsetY;
            }
            else
            {
                const float amountToErode = std::min(
                    (sedimentCapacity - sediment) * m_erosionStrength, -deltaHeight);

                const auto& brushIndices = m_brushIndices[dropletIndex];
                const auto& brushWeights = m_brushWeights[dropletIndex];

                for (std::size_t b = 0; b < brushIndices.size(); ++b)
                {
                    const int nodeIndex = brushIndices[b];
                    const float weighedErodeAmount = amountToErode * brushWeights[b];
                    const float deltaSediment = (heights[nodeIndex] < weighedErodeAmount)
                        ? heights[nodeIndex]
                        : weighedErodeAmount;
                    heights[nodeIndex] -= deltaSediment;
                    sediment += deltaSediment;
                }
            }

            speed = std::sqrt(std::max(0.0f, speed * speed + deltaHeight * m_gravity));
            water *= (1.0f - m_evaporationRate);
        }
    }

    const auto [minIt, maxIt] = std::minmax_element(heights.begin(), heights.end());
    heightField.minHeight = *minIt;
    heightField.maxHeight = *maxIt;

    std::cout << "Hydraulic erosion applied (" << m_iterations << " droplets). "
              << "New min height: " << heightField.minHeight
              << ", new max height: " << heightField.maxHeight << std::endl;
}