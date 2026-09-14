// ChunkBoundaryRenderer.cpp
#include "ChunkBoundaryRenderer.hpp"

#include <algorithm>

ChunkBoundaryRenderer::ChunkBoundaryRenderer(std::shared_ptr<Shader> shader)
    : m_shader(std::move(shader))
{
}

void ChunkBoundaryRenderer::build(const std::vector<TerrainHeightField>& chunkHeightFields, float verticalOffset)
{
    std::vector<LineVertex> vertices;

    auto sampleHeight = [](const TerrainHeightField& hf, int x, int z) -> float
    {
        x = std::clamp(x, 0, hf.pointsPerAxis - 1);
        z = std::clamp(z, 0, hf.pointsPerAxis - 1);
        return hf.heights[static_cast<std::size_t>(z) * hf.pointsPerAxis + x];
    };

    auto addSegment = [&](const TerrainHeightField& hf, int x0, int z0, int x1, int z1)
    {
        glm::vec3 p0(hf.origin.x + x0 * hf.cellSize, sampleHeight(hf, x0, z0) + verticalOffset, hf.origin.y + z0 * hf.cellSize);
        glm::vec3 p1(hf.origin.x + x1 * hf.cellSize, sampleHeight(hf, x1, z1) + verticalOffset, hf.origin.y + z1 * hf.cellSize);
        vertices.push_back({ p0 });
        vertices.push_back({ p1 });
    };

    for (const auto& hf : chunkHeightFields)
    {
        const int last = hf.pointsPerAxis - 1;

        for (int x = 0; x < last; ++x)
        {
            addSegment(hf, x, 0, x + 1, 0);       
            addSegment(hf, x, last, x + 1, last); 
        }
        for (int z = 0; z < last; ++z)
        {
            addSegment(hf, 0, z, 0, z + 1);       
            addSegment(hf, last, z, last, z + 1); 
        }
    }

    m_vertexCount = static_cast<uint32_t>(vertices.size());
    if (m_vertexCount == 0)
        return;

    auto vertexBuffer = std::make_shared<VertexBuffer>(
        vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(LineVertex)));
    vertexBuffer->setLayout({ { "aPosition", GL_FLOAT, 3, false } });

    m_vertexArray = std::make_unique<VertexArray>();
    m_vertexArray->addVertexBuffer(vertexBuffer);
}

void ChunkBoundaryRenderer::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& color) const
{
    if (!visible || !m_vertexArray || m_vertexCount == 0)
        return;

    glDisable(GL_DEPTH_TEST); // linie zawsze widoczne na wierzchu - usuń tę linię (i glEnable niżej), jeśli wolisz żeby chowały się za terenem

    m_shader->bind();
    m_shader->setMat4("uView", view);
    m_shader->setMat4("uProjection", projection);
    m_shader->setVec3("uColor", color);

    m_vertexArray->bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertexCount));
    m_vertexArray->unbind();

    glEnable(GL_DEPTH_TEST);
}