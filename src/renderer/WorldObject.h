#pragma once

#include <memory>
#include <vector>

#include "Mesh.h"
#include "Shader.h"
#include "Transform.hpp"
#include "Material.h"
#include "Light.h"

class WorldObject
{
    public:
        enum class SuperType
        {
            Default,
            Terrain
        };


        WorldObject(std::shared_ptr<Mesh> mesh,
                    std::shared_ptr<Shader> shader,
                    const std::string& name = "NEW_OBJECT",
                    const Transform& transform = Transform(),
                    std::shared_ptr<Material> material = nullptr,
                    SuperType superType = SuperType::Default);
        ~WorldObject();

        inline Transform&           transform() { return m_transform; }
        inline const std::string&   name() const { return m_name; }

        inline void setMaterial(std::shared_ptr<Material> material) { m_material = material; }
        inline std::shared_ptr<Material> material() const { return m_material; }

        inline void setSuperType(SuperType superType) { m_superType = superType; }
        inline SuperType superType() const { return m_superType; }

        inline void setMesh(std::shared_ptr<Mesh> mesh) {m_mesh = mesh;}

        void draw(const glm::mat4& projection,
                  const glm::mat4& view,
                  const glm::vec3& cameraPos,
                  const std::vector<Light>& lights) const;


    private:
        Transform                 m_transform;
        std::string               m_name;
        std::shared_ptr<Mesh>     m_mesh;
        std::shared_ptr<Shader>   m_shader;
        std::shared_ptr<Material> m_material;
        SuperType                 m_superType = SuperType::Default;
};
