#include "TerrainWorldGenerator.hpp"
#include "TerrainPipeline.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include<renderer/Mesh.h>
#include<renderer/Texture.h>

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


TerrainHeightField TerrainWorldGenerator::generateMasterHeightField(const NoiseGenerator& noise, const WorldGenerationParams& params) const
{
    const int resolution = params.chunksX * params.chunkResolution;

    TerrainPipeline pipeline;
    TerrainHeightField heightField = pipeline.run(noise, params.worldOrigin, params.chunkWorldSize * params.chunksX, resolution);

    return heightField;

}

Mesh TerrainWorldGenerator::generateMasterMesh(const NoiseGenerator& noise, const WorldGenerationParams& params, std::vector<std::shared_ptr<Texture>> textures) const
{

    TerrainHeightField heightField = generateMasterHeightField(noise, params);
    const int pointsPerAxis = heightField.pointsPerAxis;
    const float cellSize    = heightField.cellSize;
    const int resolution = params.chunksX * params.chunkResolution;



    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>( pointsPerAxis) * pointsPerAxis);

    for (int z = 0; z < pointsPerAxis; ++z)
    {
        for (int x = 0; x < pointsPerAxis; ++x)
        {
            const float worldX = params.worldOrigin.x + x * cellSize;
            const float worldZ = params.worldOrigin.y + z * cellSize;
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

    std::cout << "Generated using worldGenerator" << std::endl;
    return Mesh(vertices, indices, std::move(textures));

}

std::vector<Mesh> TerrainWorldGenerator::sliceInChunks(const NoiseGenerator& noise, const WorldGenerationParams& params,std::vector<TerrainHeightField>* outHeightFields, std::vector<std::shared_ptr<Texture>> textures) const
{
    TerrainHeightField master = generateMasterHeightField(noise, params);

    const int res = params.chunkResolution;
    const int pointsPerChunkAxis = res + 1;
    const int stride = res;

    std::vector<Mesh> chunks;
    chunks.reserve(static_cast<std::size_t>(params.chunksX) * params.chunksZ);

    for (int cz = 0; cz < params.chunksZ; ++cz)
    {
        for (int cx = 0; cx < params.chunksX; ++cx)
        {
            TerrainHeightField heightField;
            heightField.pointsPerAxis = pointsPerChunkAxis;
            heightField.cellSize = master.cellSize; 
            heightField.origin = params.worldOrigin + glm::vec2(cx, cz) * static_cast<float>(stride) * master.cellSize;

            std::vector<float> values(static_cast<std::size_t>(pointsPerChunkAxis) * pointsPerChunkAxis);

            const int baseX = cx * stride;
            const int baseZ = cz * stride;

            for (int z = 0; z < pointsPerChunkAxis; ++z)
            {
                for (int x = 0; x < pointsPerChunkAxis; ++x)
                {
                    const int masterIndex = (baseZ + z) * master.pointsPerAxis + (baseX + x);
                    values[static_cast<std::size_t>(z) * pointsPerChunkAxis + x] = master.heights[static_cast<std::size_t>(masterIndex)];
                }
            }

            heightField.heights = std::move(values);

            if(outHeightFields) outHeightFields->push_back(heightField);

            std::vector<Vertex> vertices;
            vertices.reserve(static_cast<std::size_t>(pointsPerChunkAxis) * pointsPerChunkAxis);

            for (int z = 0; z < pointsPerChunkAxis; ++z)
            {
                for (int x = 0; x < pointsPerChunkAxis; ++x)
                {
                    const float worldX = heightField.origin.x + x * heightField.cellSize;
                    const float worldZ = heightField.origin.y + z * heightField.cellSize;
                    const float height = heightField.heights[static_cast<std::size_t>(z) * pointsPerChunkAxis + x];
                    const glm::vec3 normal = estimateNormalFromHeightField(heightField, x, z);
                    const glm::vec2 uv(worldX * 0.1f, worldZ * 0.1f);

                    vertices.push_back(Vertex{ glm::vec3(worldX, height, worldZ), normal, uv, glm::vec3(1.0f) });
                }
            }

            std::vector<uint32_t> indices;
            indices.reserve(static_cast<std::size_t>(res) * res * 6);

            for (int z = 0; z < res; ++z)          
            {
                for (int x = 0; x < res; ++x)
                {
                    uint32_t topLeft = static_cast<uint32_t>(z * pointsPerChunkAxis + x);
                    uint32_t topRight = topLeft + 1;
                    uint32_t bottomLeft = static_cast<uint32_t>((z + 1) * pointsPerChunkAxis + x);
                    uint32_t bottomRight = bottomLeft + 1;

                    indices.push_back(topLeft);
                    indices.push_back(bottomLeft);
                    indices.push_back(topRight);

                    indices.push_back(topRight);
                    indices.push_back(bottomLeft);
                    indices.push_back(bottomRight);
                }
            }

            chunks.push_back(Mesh(vertices, indices, textures));
            std::cout << "Chunk: " << (cx + 1) * (cz + 1) << "/" << params.chunksX * params.chunksX << " OK!" << std::endl;
        }
    }

    std::cout << "Master Splited into chunks (" << chunks.size() << ")" << std::endl;
    return chunks;
}

