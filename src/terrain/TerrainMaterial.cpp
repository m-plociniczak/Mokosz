#include "TerrainMaterial.h"



TerrainMaterial::TerrainMaterial() {};
TerrainMaterial::~TerrainMaterial() {};

void TerrainMaterial::applyTo(Shader& shader) const
{
        Material::applyTo(shader);

        auto bindTexture = [&](std::shared_ptr<Texture> texture, int unit, const char* uniformName)
        {
            if (texture)
            {
                texture->bind(unit);
                shader.setInt(uniformName, unit);
            }
        };

        bindTexture(sandTexture, 0, "uSandTexture");
        bindTexture(grassTexture, 1, "uGrassTexture");
        bindTexture(rockTexture, 2, "uRockTexture");
        bindTexture(snowTexture, 3, "uSnowTexture");

        shader.setFloat("uSandHeight", sandHeight);
        shader.setFloat("uGrassHeight", grassHeight);
        shader.setFloat("uRockSlope", rockSlope);
        shader.setFloat("uSnowHeight", snowHeight);
        shader.setFloat("uSnowBlendWidth", snowBlendWidth);

        shader.setFloat("uSandMetallic", sandMetallic);
        shader.setFloat("uGrassMetallic", grassMetallic);
        shader.setFloat("uRockMetallic", rockMetallic);
        shader.setFloat("uSnowMetallic", snowMetallic);

        shader.setFloat("uSandRoughness", sandRoughness);
        shader.setFloat("uGrassRoughness", grassRoughness);
        shader.setFloat("uRockRoughness", rockRoughness);
        shader.setFloat("uSnowRoughness", snowRoughness);
}