#include "TerrainMeshGenerator.h"

#include <algorithm>
#include <cmath>

namespace
{
    glm::vec3 estimateNormalFromHeightField(const TerrainHeightField& heightField, int x, int z)
    {
        const float cellSize = heightField.cellSize;
        const int pointsPerAxis = heightField.pointsPerAxis;

        const float heightL = heightField.heights[static_cast<std::size_t>(z) * pointsPerAxis + std::clamp(x - 1, 0, pointsPerAxis - 1)];
        const float heightR = heightField.heights[static_cast<std::size_t>(z) * pointsPerAxis + std::clamp(x + 1, 0, pointsPerAxis - 1)];
        const float heightD = heightField.heights[static_cast<std::size_t>(std::clamp(z - 1, 0, pointsPerAxis - 1)) * pointsPerAxis + x];
        const float heightU = heightField.heights[static_cast<std::size_t>(std::clamp(z + 1, 0, pointsPerAxis - 1)) * pointsPerAxis + x];

        glm::vec3 tangentX(2.0f * cellSize, heightR - heightL, 0.0f);
        glm::vec3 tangentZ(0.0f, heightU - heightD, 2.0f * cellSize);

        return glm::normalize(glm::cross(tangentZ, tangentX));
    }
}

Mesh TerrainMeshGenerator::generatePatch(const NoiseGenerator& noise,
                                          const glm::vec2& origin,
                                          float size,
                                          int resolution,
                                          std::vector<std::shared_ptr<Texture>> textures)
{
    TerrainPipeline pipeline;
    const TerrainHeightField heightField = pipeline.run(noise, origin, size, resolution);

    const int pointsPerAxis = heightField.pointsPerAxis;
    const float cellSize = heightField.cellSize;

    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(pointsPerAxis) * pointsPerAxis);

    for (int z = 0; z < pointsPerAxis; ++z)
    {
        for (int x = 0; x < pointsPerAxis; ++x)
        {
            const float worldX = origin.x + x * cellSize;
            const float worldZ = origin.y + z * cellSize;
            const float height = heightField.heights[static_cast<size_t>(z) * pointsPerAxis + x];
            const glm::vec3 normal = estimateNormalFromHeightField(heightField, x, z);
            const glm::vec2 uv(worldX * 0.1f, worldZ * 0.1f);

            vertices.push_back(Vertex{
                glm::vec3(worldX, height, worldZ),
                normal,
                uv,
                glm::vec3(1.0f)
            });
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(resolution) * resolution * 6);

    for (int z = 0; z < resolution; ++z)
    {
        for (int x = 0; x < resolution; ++x)
        {
            uint32_t topLeft = static_cast<uint32_t>(z * pointsPerAxis + x);
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = static_cast<uint32_t>((z + 1) * pointsPerAxis + x);
            uint32_t bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return Mesh(vertices, indices, std::move(textures));
}
