#pragma once

#include <unordered_map>

#include "TerrainPipeline.h"
#include "TerrainHeightField.h"
#include<renderer/Mesh.h>

struct WorldGenerationParams
{
    int chunksX             = 4;
    int chunksZ             = 4;
    int chunkResolution     = 512;
    float chunkWorldSize    = 100.f;
    glm::vec2 worldOrigin   = glm::vec2(0.0f);

    inline float cellSize() const {return chunkWorldSize / static_cast<float>(chunkResolution);}
};




class TerrainWorldGenerator
{
public:
    Mesh generateMasterMesh(const NoiseGenerator& noise, const WorldGenerationParams& params, std::vector<std::shared_ptr<Texture>> textures = {}) const;
    TerrainHeightField generateMasterHeightField(const NoiseGenerator& noise, const WorldGenerationParams& params) const;
    
    std::vector<Mesh> sliceInChunks(const NoiseGenerator& noise, const WorldGenerationParams& params,std::vector<TerrainHeightField>* outHeightFields = nullptr ,std::vector<std::shared_ptr<Texture>> textures = {}) const;
    std::vector<std::vector<Mesh>> sliceInChunksWithLods(const NoiseGenerator& noise, const WorldGenerationParams& params, const std::vector<int>& lodStrides, std::vector<TerrainHeightField>* outHeightFields = nullptr ,std::vector<std::shared_ptr<Texture>> textures = {}) const; 
    

private:
    Mesh buildChunkMesh(const TerrainHeightField& chunkHeightField, int stride, std::vector<std::shared_ptr<Texture>> textures) const;
};