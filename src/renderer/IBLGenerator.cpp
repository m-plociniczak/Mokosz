#include "IBLGenerator.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "VertexArray.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

std::unique_ptr<Cubemap> IBLGenerator::equirectangularToCubemap(const HDRTexture& hdrEquirect, int faceSize)
{
    auto cubemap = std::make_unique<Cubemap>(faceSize, GL_RGBA16F, 1);
    Framebuffer framebuffer(faceSize, faceSize);

    Shader conversionShader("assets/shaders/equirect_to_cubemap.vert", "assets/shaders/equirect_to_cubemap.frag");
    Mesh cube = Mesh::createColoredCube();

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    glm::mat4 views[6] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    };

    conversionShader.bind();
    conversionShader.setInt("uEquirectangularMap", 0);
    conversionShader.setMat4("uProjection", projection);
    hdrEquirect.bind(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, faceSize, faceSize);

    for (unsigned int face = 0; face < 6; ++face)
    {
        conversionShader.setMat4("uView", views[face]);
        framebuffer.attachCubemapFace(cubemap->id(), face);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.draw();
    }

    Framebuffer::unbind();
    glEnable(GL_DEPTH_TEST);

    cubemap->generateMipmaps();

    return cubemap;
}

std::unique_ptr<Cubemap> IBLGenerator::convolveIrradiance(const Cubemap& environmentCubemap, int faceSize)
{
    auto irradianceMap = std::make_unique<Cubemap>(faceSize, GL_RGBA16F, 1);
    Framebuffer framebuffer(faceSize, faceSize);

    Shader convolutionShader("assets/shaders/equirect_to_cubemap.vert", "assets/shaders/irradiance_convolution.frag");
    Mesh cube = Mesh::createColoredCube();

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 views[6] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    };

    convolutionShader.bind();
    convolutionShader.setInt("uEnvironmentMap", 0);
    convolutionShader.setMat4("uProjection", projection);
    environmentCubemap.bind(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, faceSize, faceSize);

    for (unsigned int face = 0; face < 6; ++face)
    {
        convolutionShader.setMat4("uView", views[face]);
        framebuffer.attachCubemapFace(irradianceMap->id(), face);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.draw();
    }

    Framebuffer::unbind();
    glEnable(GL_DEPTH_TEST);

    return irradianceMap;
}

std::unique_ptr<Cubemap> IBLGenerator::prefilterEnvironment(const Cubemap& environmentCubemap, int baseFaceSize, int mipLevels)
{
    auto prefilterMap = std::make_unique<Cubemap>(baseFaceSize, GL_RGBA16F, mipLevels);
    Framebuffer framebuffer(baseFaceSize, baseFaceSize);

    Shader prefilterShader("assets/shaders/equirect_to_cubemap.vert", "assets/shaders/prefilter.frag");
    Mesh cube = Mesh::createColoredCube();

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 views[6] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    };

    prefilterShader.bind();
    prefilterShader.setInt("uEnvironmentMap", 0);
    prefilterShader.setMat4("uProjection", projection);
    environmentCubemap.bind(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    for (int mip = 0; mip < mipLevels; ++mip)
    {
        int mipSize = static_cast<int>(baseFaceSize * std::pow(0.5, mip));
        mipSize = std::max(mipSize, 1);
        framebuffer.resize(mipSize, mipSize);
        glViewport(0, 0, mipSize, mipSize);

        float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);
        prefilterShader.setFloat("uRoughness", roughness);

        for (unsigned int face = 0; face < 6; ++face)
        {
            prefilterShader.setMat4("uView", views[face]);
            framebuffer.attachCubemapFace(prefilterMap->id(), face, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube.draw();
        }
    }

    Framebuffer::unbind();
    glEnable(GL_DEPTH_TEST);

    return prefilterMap;
}

Texture IBLGenerator::generateBRDFLUT(int size)
{
    Texture lut = Texture::createEmpty2D(size, size, GL_RG16F);
    Framebuffer framebuffer(size, size);

    Shader brdfShader("assets/shaders/brdf_lut.vert", "assets/shaders/brdf_lut.frag");

    struct QuadVertex { glm::vec2 position; glm::vec2 texCoords; };
    std::vector<QuadVertex> quadVertices = {
        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}},
    };
    std::vector<uint32_t> quadIndices = { 0, 1, 2, 0, 2, 3 };

    VertexArray quadVAO;
    auto quadVBO = std::make_shared<VertexBuffer>(quadVertices.data(),
                                                    static_cast<uint32_t>(quadVertices.size() * sizeof(QuadVertex)));
    quadVBO->setLayout({
        { "aPosition",  GL_FLOAT, 2, false },
        { "aTexCoords", GL_FLOAT, 2, false },
    });
    auto quadEBO = std::make_shared<IndexBuffer>(quadIndices.data(), static_cast<uint32_t>(quadIndices.size()));
    quadVAO.addVertexBuffer(quadVBO);
    quadVAO.setIndexBuffer(quadEBO);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, size, size);
    framebuffer.attachTexture2D(lut.id());

    glClear(GL_COLOR_BUFFER_BIT);
    brdfShader.bind();
    quadVAO.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quadIndices.size()), GL_UNSIGNED_INT, nullptr);
    quadVAO.unbind();

    Framebuffer::unbind();
    glEnable(GL_DEPTH_TEST);

    return lut;
}
