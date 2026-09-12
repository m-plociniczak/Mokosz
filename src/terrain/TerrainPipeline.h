#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "NoiseGenerator.h"
#include "TerrainHeightField.h"
#include "TerrainPipelineStage.h"

class TerrainPipeline
{
public:
    TerrainPipeline();

    TerrainHeightField run(const NoiseGenerator& noise,
                          const glm::vec2& origin,
                          float size,
                          int resolution,
                          const std::vector<std::shared_ptr<TerrainPipelineStage>>& stages = {});

private:
    std::vector<std::shared_ptr<TerrainPipelineStage>> m_stages;
};
