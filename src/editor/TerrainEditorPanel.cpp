#include "TerrainEditorPanel.h"
#include "../terrain/TerrainMeshGenerator.h"
#include "../terrain/TerrainPipeline.h"

#include <imgui.h>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <vector>

TerrainEditorPanel::TerrainEditorPanel(std::shared_ptr<Mesh> terrainMesh,
                                        const glm::vec2& origin,
                                        float size,
                                        int resolution,
                                        const NoiseGenerator::Settings& initialSettings)
    : m_terrainMesh(std::move(terrainMesh))
    , m_noise(initialSettings)
    , m_settings(initialSettings)
    , m_origin(origin)
    , m_size(size)
    , m_resolution(resolution)
{
    regenerate();
}

void TerrainEditorPanel::regenerate()
{
    m_noise.setSettings(m_settings);
    *m_terrainMesh = TerrainMeshGenerator::generatePatch(m_noise, m_origin, m_size, m_resolution);
    refreshHeightMapTexture();
}

void TerrainEditorPanel::refreshHeightMapTexture()
{
    TerrainPipeline pipeline;
    const TerrainHeightField heightField = pipeline.run(m_noise, m_origin, m_size, m_resolution);

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
    ImGui::SliderInt("Resolution", &m_resolution, 16, 512);
    ImGui::SliderInt("Octaves", &m_settings.octaves, 1, 32);
    ImGui::SliderFloat("Persistence", &m_settings.persistence, 0.1f, 1.0f);
    ImGui::SliderFloat("Lacunarity", &m_settings.lacunarity, 1.0f, 4.0f);
    ImGui::SliderFloat("Scale", &m_settings.scale, 8.0f, 512.0f);
    ImGui::SliderFloat("Amplitude", &m_settings.amplitude, 1.0f, 100.0f);
    ImGui::SliderFloat("Island radius", &m_settings.islandRadius, 0.0f, 200.0f);
    ImGui::SliderFloat("Edge falloff", &m_settings.edgeFalloff, 0.0f, 100.0f);
    ImGui::SliderFloat("Edge drop", &m_settings.edgeDrop, -50.0f, 50.0f);
    ImGui::SliderFloat("Gradient trick strength", &m_settings.gradientTrickStrength, 0.0f, 1.0f);

    int noiseTypeIndex = static_cast<int>(m_settings.noiseType);
    if (ImGui::Combo("Noise type", &noiseTypeIndex,
                     "Plain Perlin\0Gradient Trick Perlin\0"))
    {
        m_settings.noiseType = static_cast<NoiseGenerator::Settings::NoiseType>(noiseTypeIndex);
    }

    ImGui::SliderFloat2("Seed offset", &m_settings.seedOffset.x, -1000.0f, 1000.0f);

    ImGui::Separator();
    ImGui::TextUnformatted("Erosion controls");
    ImGui::Checkbox("Enable erosion", &m_settings.enableErosion);

    if (m_settings.enableErosion)
    {
        ImGui::SliderInt("Iterations", &m_settings.iterations, 0, 10000000);
        ImGui::SliderInt("Droplet lifetime", &m_settings.dropletLifetime, 4, 60);
        ImGui::SliderFloat("Brush radius", &m_settings.brushRadius, 1.0f, 8.0f);
        ImGui::SliderFloat("Erosion strength", &m_settings.erosionStrength, 0.0f, 2.0f);
        ImGui::SliderFloat("Sediment capacity", &m_settings.sedimentCapacity, 0.0f, 2.0f);
        ImGui::SliderFloat("Deposition speed", &m_settings.depositionSpeed, 0.0f, 0.5f);
        ImGui::SliderFloat("Evaporation rate", &m_settings.evaporationRate, 0.0f, 0.2f);
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
        regenerate();
    }
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
                     ImVec2(0.0f, 0.0f),
                     ImVec2(1.0f, 1.0f));
    }
    else
    {
        ImGui::TextDisabled("Heightmap preview unavailable");
    }

    ImGui::End();
}
