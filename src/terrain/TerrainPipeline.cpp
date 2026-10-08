#include "TerrainPipeline.h"

#include "CreateNoiseMapObject.h"
#include "ApplyIslandMask.h"
#include "ApplyErosion.h"
#include "NomralizeHeightMap.hpp"
#include "ApplyAmplitude.hpp"

#include <vector>

TerrainPipeline::TerrainPipeline(const NoiseGenerator& noise)
{
    m_stages.push_back(std::make_shared<CreateNoiseMapObject>(noise));
    m_stages.push_back(std::make_shared<NomralizeHeightMap>());
    m_stages.push_back(std::make_shared<ApplyIslandMask>());
    m_stages.push_back(std::make_shared<HydraulicErosion>());
    m_stages.push_back(std::make_shared<ApplyAmplitude>());
}

TerrainHeightField TerrainPipeline::run(const NoiseGenerator& noise,
                                        const glm::vec2& origin,
                                        float size,
                                        int resolution,
                                        const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages)
{
    TerrainHeightField heightField;
    heightField.origin = origin;
    heightField.size = size;
    heightField.resolution = resolution;
    heightField.pointsPerAxis = resolution + 1;
    heightField.cellSize = size / static_cast<float>(resolution);
    heightField.heights.resize(static_cast<std::size_t>(heightField.pointsPerAxis) * heightField.pointsPerAxis, 0.0f);

    const std::vector<std::shared_ptr<TerrainPipelineStage>> activeStages = stages.empty() ? m_stages : stages;

    for (const auto& stage : activeStages)
    {
        if (auto noiseStage = std::dynamic_pointer_cast<CreateNoiseMapObject>(stage))
            noiseStage->setNoiseGenerator(noise);
        stage->apply(heightField);
    }

    return heightField;
}
