#pragma once

#include <memory>

#include "../renderer/Material.h"
#include "../renderer/Texture.h"

class TerrainMaterial : public Material
{
public:
    std::shared_ptr<Texture> sandTexture;
    std::shared_ptr<Texture> grassTexture;
    std::shared_ptr<Texture> rockTexture;
    std::shared_ptr<Texture> snowTexture;

    float sandHeight = -10.0f;
    float grassHeight = -5.0f;
    float rockSlope = 0.35f;
    float snowHeight = 11.0f;
    float snowBlendWidth = 2.0f;

    float sandMetallic = 0.0f;
    float grassMetallic = 0.0f;
    float rockMetallic = 0.1f;
    float snowMetallic = 0.35f;

    float sandRoughness = 0.95f;
    float grassRoughness = 0.85f;
    float rockRoughness = 0.70f;
    float snowRoughness = 0.25f;

    TerrainMaterial();
    ~TerrainMaterial() override;

    void applyTo(Shader& shader) const override;
   
};
