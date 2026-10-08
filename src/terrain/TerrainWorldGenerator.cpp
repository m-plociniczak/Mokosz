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


Mesh TerrainWorldGenerator::buildChunkMesh(const TerrainHeightField& chunkHeightField, int stride, std::vector<std::shared_ptr<Texture>> textures) const
{
    const int   fullPointsPerAxis  =  chunkHeightField.pointsPerAxis;
    const int   lodPointsPerAxis   =  (fullPointsPerAxis - 1) / stride + 1;
    const float lodCellSize        =  chunkHeightField.cellSize * static_cast<float>(stride);

    auto sampleHeight = [&](int lx, int lz) -> float
    {
        const int fx = std::min(lx * stride, fullPointsPerAxis - 1);
        const int fz = std::min(lz * stride, fullPointsPerAxis - 1);
        return chunkHeightField.heights[static_cast<std::size_t>(fz) * fullPointsPerAxis + fx];
    };

    auto estimateNormal = [&](int lx, int lz) -> glm::vec3
    {
        const int lxL = std::clamp(lx - 1, 0, lodPointsPerAxis - 1);
        const int lxR = std::clamp(lx + 1, 0, lodPointsPerAxis - 1);
        const int lzD = std::clamp(lz - 1, 0, lodPointsPerAxis - 1);
        const int lzU = std::clamp(lz + 1, 0, lodPointsPerAxis - 1);

        glm::vec3 tangentX(2.0f * lodCellSize, sampleHeight(lxR, lz) - sampleHeight(lxL, lz), 0.0f);
        glm::vec3 tangentZ(0.0f, sampleHeight(lx, lzU) - sampleHeight(lx, lzD), 2.0f * lodCellSize);
        return glm::normalize(glm::cross(tangentZ, tangentX));
    };

    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<std::size_t>(lodPointsPerAxis) * lodPointsPerAxis);

    for (int lz = 0; lz < lodPointsPerAxis; lz++)
    {
        for (int lx = 0; lx < lodPointsPerAxis; lx++)
        {
            const float     worldX = chunkHeightField.origin.x + lx * lodCellSize;
            const float     worldZ = chunkHeightField.origin.y + lz * lodCellSize;
            const float     height = sampleHeight(lx,lz);
            const glm::vec2 uv     = glm::vec2(worldX * 0.1f, worldZ * 0.1f);

            vertices.push_back(Vertex{ glm::vec3(worldX, height, worldZ), estimateNormal(lx, lz), uv, glm::vec3(1.0f) });
        }
        
    }

    std::vector<uint32_t> indices;
    const int quadsPerAxis = lodPointsPerAxis - 1;
    indices.reserve(static_cast<std::size_t>(quadsPerAxis) * quadsPerAxis * 6);

    for (int z = 0; z < quadsPerAxis; ++z)
    {
        for (int x = 0; x < quadsPerAxis; ++x)
        {
            uint32_t topLeft = static_cast<uint32_t>(z * lodPointsPerAxis + x);
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = static_cast<uint32_t>((z + 1) * lodPointsPerAxis + x);
            uint32_t bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return Mesh(vertices, indices, textures);
}
    



TerrainHeightField TerrainWorldGenerator::generateMasterHeightField(
    const NoiseGenerator& noise,
    const WorldGenerationParams& params,
    const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages) const
{
    const int resolution = params.chunksX * params.chunkResolution;

    TerrainPipeline pipeline(noise);
    TerrainHeightField heightField = pipeline.run(
        noise, params.worldOrigin, params.chunkWorldSize * params.chunksX, resolution, stages);

    return heightField;

}

Mesh TerrainWorldGenerator::generateMasterMesh(
    const NoiseGenerator& noise,
    const WorldGenerationParams& params,
    std::vector<std::shared_ptr<Texture>> textures,
    const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages) const
{

    TerrainHeightField heightField = generateMasterHeightField(noise, params, stages);
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

std::vector<Mesh> TerrainWorldGenerator::sliceInChunks(
    const NoiseGenerator& noise,
    const WorldGenerationParams& params,
    std::vector<TerrainHeightField>* outHeightFields,
    std::vector<std::shared_ptr<Texture>> textures,
    const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages) const
{
    TerrainHeightField master = generateMasterHeightField(noise, params, stages);

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

            chunks.push_back(buildChunkMesh(heightField, 1, textures));
            std::cout << "Chunk: " << (cx + 1) * (cz + 1) << "/" << params.chunksX * params.chunksX << " OK!" << std::endl;
        }
    }

    std::cout << "Master Splited into chunks (" << chunks.size() << ")" << std::endl;
    return chunks;
}


std::vector<std::vector<Mesh>> TerrainWorldGenerator::sliceInChunksWithLods(
    const NoiseGenerator& noise,
    const WorldGenerationParams& params,
    const std::vector<int>& lodStrides,
    std::vector<TerrainHeightField>* outHeightFields,
    std::vector<std::shared_ptr<Texture>> textures,
    const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages) const
{
    TerrainHeightField master = generateMasterHeightField(noise, params, stages);

    const int res = params.chunkResolution;
    const int pointsPerChunkAxis = res + 1;
    const int stride = res;

    std::vector<std::vector<Mesh>> chunkLodMeshes;
    chunkLodMeshes.reserve(static_cast<std::size_t>(params.chunksX) * params.chunksZ);

    for (int cz = 0; cz < params.chunksZ; ++cz)
    {
        for (int cx = 0; cx < params.chunksX; ++cx)
        {
            TerrainHeightField heightField;
            heightField.pointsPerAxis   = pointsPerChunkAxis;
            heightField.cellSize        = master.cellSize;
            heightField.origin          = params.worldOrigin + glm::vec2(cx, cz) * static_cast<float>(stride) * master.cellSize;

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
            if (outHeightFields) outHeightFields->push_back(heightField);

            std::vector<Mesh> lodMeshes;
            lodMeshes.reserve(lodStrides.size());

            for (int lodStride : lodStrides)
            {
                if (res % lodStride != 0)
                {
                    std::cerr << "sliceInChunksWithLods: chunkResolution (" << res
                              << ") not devilable by 2 " << lodStride
                              << "  skiping for chunk (" << cx << ", " << cz << ")" << std::endl;
                    continue;
                }

                lodMeshes.push_back(buildChunkMesh(heightField, lodStride, textures));
            }

            const int chunkIndex = cz * params.chunksX + cx;
            const int totalChunks = params.chunksX * params.chunksZ;
            std::cout << "Chunk: " << (chunkIndex + 1) << "/" << totalChunks << " OK! (" << lodMeshes.size() << " LOD levels)" << std::endl;

            chunkLodMeshes.push_back(std::move(lodMeshes));

        }
    }

        std::cout << "Master Splited into chunks with LODs (" << chunkLodMeshes.size() << ")" << std::endl;
        return chunkLodMeshes;
    

}

