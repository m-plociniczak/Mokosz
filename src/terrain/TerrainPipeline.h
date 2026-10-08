#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "NoiseGenerator.h"
#include "TerrainHeightField.h"
#include "TerrainPipelineStage.h"

class TerrainPipeline
{
public:
    TerrainPipeline(const NoiseGenerator& noise);

    TerrainHeightField run(const NoiseGenerator& noise,
                          const glm::vec2& origin,
                          float size,
                          int resolution,
                          const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages = {});

    const std::vector<std::shared_ptr<TerrainPipelineStage>>& getStages() const { return m_stages; }

    template <typename StageType>
    std::shared_ptr<StageType> getStage() const
    {
        for (const auto& stage : m_stages)
        {
            if (auto typedStage = std::dynamic_pointer_cast<StageType>(stage))
                return typedStage;
        }

        throw std::out_of_range("TerrainPipeline stage not found");
    }

private:
    std::vector<std::shared_ptr<TerrainPipelineStage>> m_stages;
};
