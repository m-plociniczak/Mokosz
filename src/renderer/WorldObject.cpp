#include "WorldObject.h"
#include <glm/gtc/matrix_inverse.hpp>

namespace
{
    // Must match "#define MAX_LIGHTS 4" in pbr.frag -- the shader's
    // uLightPositions/uLightColors arrays are fixed-size, so more lights
    // than this are silently dropped rather than overflowing the uniform.
    constexpr size_t MAX_LIGHTS = 4;
}

WorldObject::WorldObject(
    std::shared_ptr<Mesh> mesh,
    std::shared_ptr<Shader> shader,
    const std::string& name,
    const Transform& transform,
    std::shared_ptr<Material> material,
    SuperType superType)
    : m_transform(transform),
      m_name(name),
      m_mesh(std::move(mesh)),
      m_shader(std::move(shader)),
      m_material(std::move(material)),
      m_superType(superType)
{
}

WorldObject::~WorldObject() {}


void WorldObject::draw(const glm::mat4& projection,
                        const glm::mat4& view,
                        const glm::vec3& cameraPos,
                        const std::vector<Light>& lights) const
{
    m_shader->bind();
    m_shader->setMat4("uProjection", projection);
    m_shader->setMat4("uView", view);

    glm::mat4 model = m_transform.getModelMatrix();
    m_shader->setMat4("uModel", model);

    if (m_material)
    {
        // Correct normal transform under non-uniform scale: glm::inverseTranspose
        // is equivalent to transpose(inverse(mat3(model))) but avoids computing
        // an unused 4th row/column.
        glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));
        m_shader->setMat3("uNormalMatrix", normalMatrix);

        m_material->applyTo(*m_shader);
        m_shader->setVec3("uCameraPos", cameraPos);

        size_t lightCount = std::min(lights.size(), MAX_LIGHTS);
        m_shader->setInt("uLightCount", static_cast<int>(lightCount));
        for (size_t i = 0; i < lightCount; ++i)
        {
            std::string index = std::to_string(i);
            m_shader->setVec3("uLightPositions[" + index + "]", lights[i].position);
            m_shader->setVec3("uLightDirections[" + index + "]", lights[i].direction);
            m_shader->setVec3("uLightColors[" + index + "]", lights[i].color);
            m_shader->setBool("uLightIsDirectional[" + index + "]", lights[i].isDirectional);
        }
    }

    m_mesh->drawWithMaterial(*m_shader);
}
