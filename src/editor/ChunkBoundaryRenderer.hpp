#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include <terrain/TerrainHeightField.h>
#include <renderer/Shader.h>
#include <renderer/VertexArray.h>

class ChunkBoundaryRenderer
{
public:
    explicit ChunkBoundaryRenderer(std::shared_ptr<Shader> shader);

    void build(const std::vector<TerrainHeightField>& chunkHeightFields, float verticalOffset = 0.05f);

    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& color = glm::vec3(1.0f, 0.0f, 1.0f)) const;

    bool visible = true;

private:
    struct LineVertex { glm::vec3 position; };

    std::unique_ptr<VertexArray> m_vertexArray;
    uint32_t m_vertexCount = 0;
    std::shared_ptr<Shader> m_shader;
};