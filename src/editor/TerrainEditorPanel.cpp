#include "TerrainEditorPanel.h"
#include "../terrain/ApplyAmplitude.hpp"
#include "../terrain/ApplyErosion.h"
#include "../terrain/ApplyIslandMask.h"
#include "../terrain/TerrainMeshGenerator.h"
#include "../terrain/TerrainPipeline.h"

#include <imgui.h>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include <iostream>

TerrainEditorPanel::TerrainEditorPanel(TerrainLodManager& lodManager,
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
                                        std::vector<std::shared_ptr<Texture>> textures,
                                        const NoiseGenerator::Settings& initialSettings)
    : m_lodManager(lodManager)
    , m_lodStrides(lodStrides)
    , m_chunkBoundaryRenderer(chunkBoundaryRenderer)
    , m_terrainCollider(terrainCollider)
    , m_textures(std::move(textures))
    , m_noise(noiseGenerator)
    , m_pipeline(pipeline)
    , m_settings(initialSettings)
    , m_params(worldGenerationParams)
    , m_worldGenerator(worldGenerator)
    , m_origin(origin)
    , m_size(size)
    , m_resolution(resolution)
{
    m_noise.setSettings(m_settings);

    auto amplitude = m_pipeline.getStage<ApplyAmplitude>();
    amplitude->setAmplitude(m_settings.amplitude);

    auto islandMask = m_pipeline.getStage<ApplyIslandMask>();
    islandMask->setIslandRadius(m_settings.islandRadius);
    islandMask->setEdgeFalloff(m_settings.edgeFalloff);
    islandMask->setSeedOffset(m_settings.seedOffset);

    auto erosion = m_pipeline.getStage<HydraulicErosion>();
    erosion->setEnableErosion(m_settings.enableErosion);
    erosion->setIterations(m_settings.iterations);
    erosion->setDropletLifetime(m_settings.dropletLifetime);
    erosion->setBrushRadius(static_cast<int>(std::lround(m_settings.brushRadius)));
    erosion->setErosionStrength(m_settings.erosionStrength);
    erosion->setSedimentCapacity(m_settings.sedimentCapacity);
    erosion->setDepositionSpeed(m_settings.depositionSpeed);
    erosion->setEvaporationRate(m_settings.evaporationRate);
    erosion->setInertia(m_settings.inertia);
    erosion->setSedimentCapacityFactor(m_settings.sedimentCapacityFactor);
    erosion->setMinSedimentCapacity(m_settings.minSedimentCapacity);
    erosion->setErodeSpeed(m_settings.erodeSpeed);
    erosion->setGravity(m_settings.gravity);
    erosion->setMaxDropletLifetime(m_settings.maxDropletLifetime);
    erosion->setInitialWaterVolume(m_settings.initialWaterVolume);
    erosion->setInitialSpeed(m_settings.initialSpeed);
    erosion->setSeedOffset(m_settings.seedOffset);
}

void TerrainEditorPanel::regenerate()
{
    m_noise.setSettings(m_settings);

    std::vector<TerrainHeightField> heightFields;
    auto chunkLodMeshesRaw = m_worldGenerator.sliceInChunksWithLods(
        m_noise, m_params, m_lodStrides, &heightFields, m_textures, m_pipeline.getStages());

    if (m_chunkBoundaryRenderer)
        m_chunkBoundaryRenderer->build(heightFields);

    if (m_terrainCollider)
        m_terrainCollider->updateChunks(heightFields);

    const std::size_t expectedCount = static_cast<std::size_t>(m_params.chunksX) * m_params.chunksZ;

    if (chunkLodMeshesRaw.size() != expectedCount || chunkLodMeshesRaw.size() != m_lodManager.chunkCount())
    {
        std::cerr << "TerrainEditorPanel::regenerate: chunk count mismatch! "
                  << "generated=" << chunkLodMeshesRaw.size()
                  << " expected=" << expectedCount
                  << " lodManager=" << m_lodManager.chunkCount()
                  <<  std::endl;
        return;
    }

    for (std::size_t i = 0; i < chunkLodMeshesRaw.size(); ++i)
    {
        const TerrainHeightField& hf = heightFields[i];
        const float chunkSize = static_cast<float>(hf.pointsPerAxis - 1) * hf.cellSize;
        const glm::vec3 boundsCenter(hf.origin.x + chunkSize * 0.5f, 0.0f, hf.origin.y + chunkSize * 0.5f);
        const float boundsRadius = chunkSize * 0.7071f;

        m_lodManager.updateChunkMeshes(i, chunkLodMeshesRaw[i], boundsCenter, boundsRadius);
    }

    refreshHeightMapTexture();
}

void TerrainEditorPanel::refreshHeightMapTexture()
{
    const TerrainHeightField heightField = m_pipeline.run(
        m_noise, m_origin, m_size, m_resolution, m_pipeline.getStages());

    const int previewSize = heightField.pointsPerAxis;

    float currentMinHeight = std::numeric_limits<float>::max();
    float currentMaxHeight = std::numeric_limits<float>::lowest();

    for (const float height : heightField.heights)
    {
        currentMinHeight = std::min(currentMinHeight, height);
        currentMaxHeight = std::max(currentMaxHeight, height);
    }

    const float heightSpan = std::max(0.0001f, currentMaxHeight - currentMinHeight);

    std::vector<float> heightMapData;
    heightMapData.reserve(static_cast<std::size_t>(previewSize) * previewSize);

    for (int z = 0; z < previewSize; ++z)
    {
        for (int x = 0; x < previewSize; ++x)
        {
            const float height = heightField.heights[static_cast<std::size_t>(z) * heightField.pointsPerAxis + x];
            const float normalized = std::clamp((height - currentMinHeight) / heightSpan, 0.0f, 1.0f);
            heightMapData.push_back(normalized);
        }
    }

    m_heightMapTexture = std::make_unique<Texture>(Texture::createFromFloatData(previewSize, previewSize, heightMapData));
}

void TerrainEditorPanel::drawNested()
{
    ImGui::Begin("Terrain generator");

    ImGui::SliderInt("Resolution", &m_params.chunkResolution, 8, 512);
    ImGui::SliderFloat("Chunk world size", &m_params.chunkWorldSize, 1.0f, 1000.0f);
    ImGui::SliderInt("Chunks per axis", &m_params.chunksX, 1, 64);
    ImGui::SliderFloat2("World origin", &m_params.worldOrigin.x, -1000.0f, 1000.0f);
    m_params.chunksZ = m_params.chunksX;

    if (ImGui::TreeNode("1. Create noise map"))
    {
        ImGui::SliderInt("Octaves", &m_settings.octaves, 1, 32);
        ImGui::SliderFloat("Persistence", &m_settings.persistence, 0.1f, 1.0f);
        ImGui::SliderFloat("Lacunarity", &m_settings.lacunarity, 1.0f, 4.0f);
        ImGui::SliderFloat("Scale", &m_settings.scale, 8.0f, 512.0f);
        ImGui::SliderFloat("Gradient trick strength", &m_settings.gradientTrickStrength, 0.0f, 1.0f);

        int noiseTypeIndex = static_cast<int>(m_settings.noiseType);
        if (ImGui::Combo("Noise type", &noiseTypeIndex,
                         "Plain Perlin\0Gradient Trick Perlin\0Domain Warped Perlin\0"))
        {
            m_settings.noiseType = static_cast<NoiseGenerator::Settings::NoiseType>(noiseTypeIndex);
        }

        if (ImGui::SliderFloat2("Seed offset", &m_settings.seedOffset.x, -1000.0f, 1000.0f))
        {
            m_pipeline.getStage<ApplyIslandMask>()->setSeedOffset(m_settings.seedOffset);
            m_pipeline.getStage<HydraulicErosion>()->setSeedOffset(m_settings.seedOffset);
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("2. Normalize height map"))
    {
        ImGui::TextDisabled("Normalizes the generated noise height values.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("3. Apply island mask"))
    {
        auto islandMask = m_pipeline.getStage<ApplyIslandMask>();
        float islandRadius = islandMask->getIslandRadius();
        if (ImGui::SliderFloat("Island radius", &islandRadius, 0.0f, 500.0f))
            islandMask->setIslandRadius(islandRadius);
        float edgeFalloff = islandMask->getEdgeFalloff();
        if (ImGui::SliderFloat("Edge falloff", &edgeFalloff, 0.1f, 200.0f))
            islandMask->setEdgeFalloff(edgeFalloff);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("4. Hydraulic erosion"))
    {
        auto erosion = m_pipeline.getStage<HydraulicErosion>();
        bool erosionEnabled = erosion->getEnableErosion();
        if (ImGui::Checkbox("Enable erosion", &erosionEnabled))
            erosion->setEnableErosion(erosionEnabled);

        if (erosionEnabled)
        {
            int iterations = erosion->getIterations();
            if (ImGui::SliderInt("Iterations", &iterations, 0, 100000))
                erosion->setIterations(iterations);
            int lifetime = erosion->getDropletLifetime();
            if (ImGui::SliderInt("Droplet lifetime", &lifetime, 4, 60))
                erosion->setDropletLifetime(lifetime);
            int brushRadius = erosion->getBrushRadius();
            if (ImGui::SliderInt("Brush radius", &brushRadius, 2, 8))
                erosion->setBrushRadius(brushRadius);

            float erosionStrength = erosion->getErosionStrength();
            if (ImGui::SliderFloat("Erosion strength", &erosionStrength, 0.0f, 2.0f))
                erosion->setErosionStrength(erosionStrength);
            float sedimentCapacity = erosion->getSedimentCapacity();
            if (ImGui::SliderFloat("Sediment capacity", &sedimentCapacity, 0.0f, 10.0f))
                erosion->setSedimentCapacity(sedimentCapacity);
            float depositionSpeed = erosion->getDepositionSpeed();
            if (ImGui::SliderFloat("Deposition speed", &depositionSpeed, 0.0f, 0.5f))
                erosion->setDepositionSpeed(depositionSpeed);
            float evaporationRate = erosion->getEvaporationRate();
            if (ImGui::SliderFloat("Evaporation rate", &evaporationRate, 0.0f, 0.2f))
                erosion->setEvaporationRate(evaporationRate);
            float inertia = erosion->getInertia();
            if (ImGui::SliderFloat("Inertia", &inertia, 0.0f, 1.0f))
                erosion->setInertia(inertia);

            if (ImGui::TreeNode("Advanced erosion"))
            {
                float capacityFactor = erosion->getSedimentCapacityFactor();
                if (ImGui::SliderFloat("Capacity factor", &capacityFactor, 0.1f, 10.0f))
                    erosion->setSedimentCapacityFactor(capacityFactor);
                float minCapacity = erosion->getMinSedimentCapacity();
                if (ImGui::SliderFloat("Minimum capacity", &minCapacity, 0.001f, 1.0f))
                    erosion->setMinSedimentCapacity(minCapacity);
                float erodeSpeed = erosion->getErodeSpeed();
                if (ImGui::SliderFloat("Erode speed", &erodeSpeed, 0.0f, 1.0f))
                    erosion->setErodeSpeed(erodeSpeed);
                float gravity = erosion->getGravity();
                if (ImGui::SliderFloat("Gravity", &gravity, 0.0f, 10.0f))
                    erosion->setGravity(gravity);
                float maxLifetime = erosion->getMaxDropletLifetime();
                if (ImGui::SliderFloat("Maximum droplet lifetime", &maxLifetime, 1.0f, 100.0f))
                    erosion->setMaxDropletLifetime(maxLifetime);
                float waterVolume = erosion->getInitialWaterVolume();
                if (ImGui::SliderFloat("Initial water volume", &waterVolume, 0.01f, 5.0f))
                    erosion->setInitialWaterVolume(waterVolume);
                float initialSpeed = erosion->getInitialSpeed();
                if (ImGui::SliderFloat("Initial speed", &initialSpeed, 0.01f, 5.0f))
                    erosion->setInitialSpeed(initialSpeed);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("5. Apply amplitude"))
    {
        auto amplitudeStage = m_pipeline.getStage<ApplyAmplitude>();
        float amplitude = amplitudeStage->getAmplitude();
        if (ImGui::SliderFloat("Amplitude", &amplitude, 1.0f, 100.0f))
            amplitudeStage->setAmplitude(amplitude);
        ImGui::TreePop();
    }

    if (m_chunkBoundaryRenderer)
    {
        ImGui::Checkbox("Show chunk boundaries", &m_chunkBoundaryRenderer->visible);
    }

    if (ImGui::Button("Regenerate terrain"))
    {
        regenerate();
    }

    ImGui::SameLine();
    if (ImGui::Button("Random seed"))
    {
        m_settings.seedOffset = glm::vec2(
            static_cast<float>(rand() % 10000),
            static_cast<float>(rand() % 10000));
        m_pipeline.getStage<ApplyIslandMask>()->setSeedOffset(m_settings.seedOffset);
        m_pipeline.getStage<HydraulicErosion>()->setSeedOffset(m_settings.seedOffset);
        regenerate();
    }

    ImGui::End();
}

void TerrainEditorPanel::draw()
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(160.0f, 160.0f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Heightmap Preview", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (m_heightMapTexture)
    {
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(m_heightMapTexture->id())),
                     ImVec2(static_cast<float>(m_heightMapTexture->width()),
                            static_cast<float>(m_heightMapTexture->height())),
                     ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
    }
    else
    {
        ImGui::TextDisabled("Heightmap preview unavailable");
    }

    ImGui::End();
}