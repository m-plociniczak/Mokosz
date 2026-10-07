#include "Mesh.h"

#include <cmath>

namespace
{
    constexpr float PI = 3.14159265358979323846f;
}

Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<uint32_t>& indices,
           std::vector<std::shared_ptr<Texture>> textures)
    : m_textures(std::move(textures))
{
    auto vertexBuffer = std::make_shared<VertexBuffer>(
        vertices.data(),
        static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));

    vertexBuffer->setLayout({
        { "aPosition",  GL_FLOAT, 3, false },
        { "aNormal",    GL_FLOAT, 3, false },
        { "aTexCoords", GL_FLOAT, 2, false },
        { "aColor",     GL_FLOAT, 3, false },
    });

    auto indexBuffer = std::make_shared<IndexBuffer>(
        indices.data(),
        static_cast<uint32_t>(indices.size()));

    m_vertexArray.addVertexBuffer(vertexBuffer);
    m_vertexArray.setIndexBuffer(indexBuffer);
}

void Mesh::draw() const
{
    m_vertexArray.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_vertexArray.indexBuffer()->count()), GL_UNSIGNED_INT, nullptr);
    m_vertexArray.unbind();
}

void Mesh::drawWithMaterial(Shader& shader) const
{
    if (m_textures.empty())
    {
        shader.setBool("uHasDiffuseTexture", false);
    }
    else
    {
        shader.setBool("uHasDiffuseTexture", true);
        m_textures[0]->bind(0);
        shader.setInt("uDiffuseTexture", 0);
    }

    draw();
}

Mesh Mesh::createColoredCube()
{
    const glm::vec3 red(0.90f, 0.20f, 0.20f);
    const glm::vec3 green(0.20f, 0.85f, 0.30f);
    const glm::vec3 blue(0.20f, 0.40f, 0.90f);
    const glm::vec3 yellow(0.95f, 0.85f, 0.20f);
    const glm::vec3 magenta(0.85f, 0.25f, 0.85f);
    const glm::vec3 cyan(0.25f, 0.85f, 0.85f);

    auto face = [](glm::vec3 n, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 col)
    {
        return std::vector<Vertex>{
            { a, n, {0.0f, 0.0f}, col },
            { b, n, {1.0f, 0.0f}, col },
            { c, n, {1.0f, 1.0f}, col },
            { d, n, {0.0f, 1.0f}, col },
        };
    };

    std::vector<Vertex> vertices;

    auto append = [&vertices](const std::vector<Vertex>& faceVerts)
    {
        vertices.insert(vertices.end(), faceVerts.begin(), faceVerts.end());
    };

    append(face({1,0,0}, {0.5f,-0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {0.5f,0.5f,0.5f}, {0.5f,-0.5f,0.5f}, red));
    append(face({-1,0,0}, {-0.5f,-0.5f,0.5f}, {-0.5f,0.5f,0.5f}, {-0.5f,0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, green));
    append(face({0,1,0}, {-0.5f,0.5f,-0.5f}, {-0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f}, {0.5f,0.5f,-0.5f}, blue));
    append(face({0,-1,0}, {-0.5f,-0.5f,0.5f}, {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,0.5f}, yellow));
    append(face({0,0,1}, {-0.5f,-0.5f,0.5f}, {0.5f,-0.5f,0.5f}, {0.5f,0.5f,0.5f}, {-0.5f,0.5f,0.5f}, magenta));
    append(face({0,0,-1}, {0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f,0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, cyan));

    std::vector<uint32_t> indices;
    indices.reserve(36);
    for (uint32_t f = 0; f < 6; ++f)
    {
        uint32_t base = f * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::createPlane(float width, float depth)
{
    const float halfWidth = width * 0.5f;
    const float halfDepth = depth * 0.5f;
    const glm::vec3 normal(0.0f, 1.0f, 0.0f);

    const std::vector<Vertex> vertices{
        { {-halfWidth, 0.0f, -halfDepth}, normal, {0.0f, 0.0f}, glm::vec3(1.0f) },
        { {-halfWidth, 0.0f,  halfDepth}, normal, {0.0f, 1.0f}, glm::vec3(1.0f) },
        { { halfWidth, 0.0f,  halfDepth}, normal, {1.0f, 1.0f}, glm::vec3(1.0f) },
        { { halfWidth, 0.0f, -halfDepth}, normal, {1.0f, 0.0f}, glm::vec3(1.0f) },
    };
    const std::vector<uint32_t> indices{ 0, 1, 2, 0, 2, 3 };

    return Mesh(vertices, indices);
}

Mesh Mesh::createUVSphere(float radius, uint32_t latSegments, uint32_t lonSegments)
{
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(latSegments + 1) * (lonSegments + 1));

    for (uint32_t lat = 0; lat <= latSegments; ++lat)
    {
        // theta: 0 at north pole, PI at south pole
        float theta = static_cast<float>(lat) / static_cast<float>(latSegments) * PI;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (uint32_t lon = 0; lon <= lonSegments; ++lon)
        {
            // phi: 0..2PI around the vertical axis
            float phi = static_cast<float>(lon) / static_cast<float>(lonSegments) * 2.0f * PI;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 normal(
                sinTheta * cosPhi,
                cosTheta,
                sinTheta * sinPhi);

            glm::vec2 uv(
                static_cast<float>(lon) / static_cast<float>(lonSegments),
                static_cast<float>(lat) / static_cast<float>(latSegments));

            vertices.push_back(Vertex{
                normal * radius,
                normal,
                uv,
                glm::vec3(1.0f)
            });
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(latSegments) * lonSegments * 6);

    uint32_t stride = lonSegments + 1;
    for (uint32_t lat = 0; lat < latSegments; ++lat)
    {
        for (uint32_t lon = 0; lon < lonSegments; ++lon)
        {
            uint32_t current = lat * stride + lon;
            uint32_t next = current + stride;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    return Mesh(vertices, indices);
}