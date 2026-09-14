#pragma once

#include <memory>

#include "../terrain/NoiseGenerator.h"
#include "../renderer/Mesh.h"
#include <terrain/TerrainWorldGenerator.hpp>
#include "ChunkBoundaryRenderer.hpp"

class TerrainEditorPanel
{
public:
    TerrainEditorPanel(std::vector<std::shared_ptr<Mesh>>& terrainChunkMeshes,
                        const glm::vec2& origin,
                        float size,
                        int resolution,
                        const WorldGenerationParams& WorldGenerationParams,
                        const TerrainWorldGenerator& worldGenerator,
                        const std::shared_ptr<ChunkBoundaryRenderer>& chunkBoundaryRenderer,
                        const NoiseGenerator::Settings& initialSettings = NoiseGenerator::Settings());

    // Render the terrain editor controls in a standalone ImGui window.
    void draw();

    // Render the terrain editor controls inside another ImGui panel.
    void drawNested();

private:
    std::shared_ptr<Mesh> m_terrainMesh;
    std::shared_ptr<ChunkBoundaryRenderer> m_chunkBoundaryRenderer;
    std::vector<std::shared_ptr<Mesh>> m_terrainChunkMeshes;
    NoiseGenerator m_noise;
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
