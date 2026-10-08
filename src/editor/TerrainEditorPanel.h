#pragma once

#include <memory>
#include <vector>

#include "../terrain/NoiseGenerator.h"
#include "../terrain/TerrainPipeline.h"
#include "../renderer/Mesh.h"
#include <terrain/TerrainWorldGenerator.hpp>
#include <terrain/TerrainLodManager.hpp>
#include "ChunkBoundaryRenderer.hpp"

class TerrainEditorPanel
{
public:
    TerrainEditorPanel(TerrainLodManager& lodManager,
                        std::shared_ptr<TerrainCollider> terrainCollider,
                        const std::vector<int>& lodStrides,
                        const glm::vec2& origin,
                        float size,
                        int resolution,
                        const WorldGenerationParams& worldGenerationParams,
                        const TerrainWorldGenerator& worldGenerator,
                        NoiseGenerator& noiseGenerator,
                        TerrainPipeline& pipeline,
                        const std::shared_ptr<ChunkBoundaryRenderer>& chunkBoundaryRenderer,
                        std::vector<std::shared_ptr<Texture>> textures = {},
                        const NoiseGenerator::Settings& initialSettings = NoiseGenerator::Settings());

    void draw();
    void drawNested();

private:
    TerrainLodManager& m_lodManager;
    std::vector<int> m_lodStrides;
    std::shared_ptr<ChunkBoundaryRenderer> m_chunkBoundaryRenderer;
    std::shared_ptr<TerrainCollider>      m_terrainCollider = nullptr;
    std::vector<std::shared_ptr<Texture>> m_textures;
    NoiseGenerator& m_noise;
    TerrainPipeline& m_pipeline;
    NoiseGenerator::Settings m_settings;
    WorldGenerationParams m_params;
    TerrainWorldGenerator m_worldGenerator;

    glm::vec2 m_origin;
    float m_size;
    int m_resolution;
    std::unique_ptr<Texture> m_heightMapTexture;

    void regenerate();
    void refreshHeightMapTexture();
};