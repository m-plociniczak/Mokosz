#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

struct TerrainHeightField
{
    glm::vec2 origin = glm::vec2(0.0f);
    float size = 0.0f;
    int resolution = 0;
    int pointsPerAxis = 0;
    float cellSize = 0.0f;
    float minHeight = 0.0f;
    float maxHeight = 0.0f;
    std::vector<float> heights;

    float getHeightAt(float worldX, float worldZ) const
    {
        if (pointsPerAxis <= 1 || cellSize <= 0.0f || heights.empty())
        {
            return 0.0f;
        }

        const float localX = std::clamp((worldX - origin.x) / cellSize,
                                        0.0f,
                                        static_cast<float>(pointsPerAxis - 1));
        const float localZ = std::clamp((worldZ - origin.y) / cellSize,
                                        0.0f,
                                        static_cast<float>(pointsPerAxis - 1));

        const int x0 = std::clamp(static_cast<int>(std::floor(localX)), 0, pointsPerAxis - 1);
        const int z0 = std::clamp(static_cast<int>(std::floor(localZ)), 0, pointsPerAxis - 1);
        const int x1 = std::clamp(x0 + 1, 0, pointsPerAxis - 1);
        const int z1 = std::clamp(z0 + 1, 0, pointsPerAxis - 1);

        const float tx = localX - static_cast<float>(x0);
        const float tz = localZ - static_cast<float>(z0);

        const float h00 = heights[static_cast<std::size_t>(z0) * pointsPerAxis + x0];
        const float h10 = heights[static_cast<std::size_t>(z0) * pointsPerAxis + x1];
        const float h01 = heights[static_cast<std::size_t>(z1) * pointsPerAxis + x0];
        const float h11 = heights[static_cast<std::size_t>(z1) * pointsPerAxis + x1];

        const float top = h00 + (h10 - h00) * tx;
        const float bottom = h01 + (h11 - h01) * tx;

        return top + (bottom - top) * tz;
    }

    glm::vec3 getNormalAt(float worldX, float worldZ) const
    {
        if (pointsPerAxis <= 1 || cellSize <= 0.0f || heights.empty())
        {
            return glm::vec3(0.0f, 1.0f, 0.0f);
        }

        const float halfStep = cellSize * 0.5f;
        const float hLeft = getHeightAt(worldX - halfStep, worldZ);
        const float hRight = getHeightAt(worldX + halfStep, worldZ);
        const float hDown = getHeightAt(worldX, worldZ - halfStep);
        const float hUp = getHeightAt(worldX, worldZ + halfStep);

        glm::vec3 tangentX(2.0f * halfStep, hRight - hLeft, 0.0f);
        glm::vec3 tangentZ(0.0f, hUp - hDown, 2.0f * halfStep);

        glm::vec3 normal = glm::normalize(glm::cross(tangentZ, tangentX));

        if (glm::dot(normal, normal) <= 0.0f)
        {
            return glm::vec3(0.0f, 1.0f, 0.0f);
        }

        return normal;
    }
};
