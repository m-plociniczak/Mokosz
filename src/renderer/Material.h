#pragma once

#include <glm/glm.hpp>
#include "Shader.h"


class Material
{


public:
    std::string name = "NEW_MATERIAL";
    glm::vec3 albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f; 

    Material() {}
    virtual ~Material(){}


    // !!! uMaterial.* uniforms. Call shader.bind() before this. !!!
    virtual void applyTo(Shader& shader) const
    {
        shader.setVec3("uMaterial.albedo", albedo);
        shader.setFloat("uMaterial.metallic", metallic);
        shader.setFloat("uMaterial.roughness", roughness);
        shader.setFloat("uMaterial.ao", ao);
    }
};
