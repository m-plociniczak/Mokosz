#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include <vector>
#include <cstdint>


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal   = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec2 texCoords = glm::vec2(0.0f);
    glm::vec3 color     = glm::vec3(1.0f);
};


class Mesh
{
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, std::vector<std::shared_ptr<Texture>> textures = {});

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept = default;
    Mesh& operator=(Mesh&& other) noexcept = default;

    void draw() const;
    void drawWithMaterial(Shader& shader) const;

    static Mesh createColoredCube();
    static Mesh createPlane(float width = 1.0f, float depth = 1.0f);
    static Mesh createUVSphere(float radius = 1.0f, uint32_t latSegments = 32, uint32_t lonSegments = 32);

private:
    VertexArray m_vertexArray;
    std::vector<std::shared_ptr<Texture>> m_textures;
};
