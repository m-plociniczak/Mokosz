#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <exception>

#include <core/Window.h>
#include <core/Camera.hpp>
#include <core/KeyInput.hpp>
#include <core/ObjectLoader.hpp>
#include <core/Scean.hpp>

#include <renderer/Shader.h>
#include <renderer/Mesh.h>
#include <renderer/WorldObject.h>
#include <renderer/Material.h>
#include <renderer/Light.h>
#include <renderer/HDRTexture.h>
#include <renderer/IBLGenerator.h>

#include <terrain/NoiseGenerator.h>
#include <terrain/TerrainMeshGenerator.h>
#include <terrain/TerrainMaterial.h>

#include <editor/EditorGUI.h>
#include <editor/TerrainEditorPanel.h>


int main()
{
    try
    {
        Window window(1280, 720, "PBR Renderer");

        float aspectRatio = static_cast<float>(window.width()) / static_cast<float>(window.height());
        window.setResizeCallback([&aspectRatio](int width, int height)
        {
            aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        });

        glEnable(GL_DEPTH_TEST);

        Scean scene;

        Light directionalLight;
        directionalLight.isDirectional = true;
        directionalLight.direction = glm::normalize(glm::vec3(1.0f, 0.1f, 0.65f));
        directionalLight.color = glm::vec3(12.0f, 2.0f, 1.0f);
        scene.addLight(directionalLight);

        std::size_t basicShaderIndex   = scene.addShader(std::make_shared<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag"));
        std::size_t pbrShaderIndex     = scene.addShader(std::make_shared<Shader>("assets/shaders/pbr.vert", "assets/shaders/pbr.frag"));
        std::size_t terrainShaderIndex = scene.addShader(std::make_shared<Shader>("assets/shaders/terrain.vert", "assets/shaders/terrain.frag"));

        std::size_t cubeMeshIndex    = scene.addMesh(std::make_shared<Mesh>(Mesh::createColoredCube()));
        std::size_t sphereMeshIndex  = scene.addMesh(std::make_shared<Mesh>(Mesh::createUVSphere(1.0f, 32, 32)));


        //std::size_t cube1Index = scene.addWorldObject(WorldObject(scene.getMesh(cubeMeshIndex), scene.getShader(basicShaderIndex), Transform(glm::vec3(2.0f, 0.0f, -1.5f))));
        //std::size_t cube2Index = scene.addWorldObject(WorldObject(scene.getMesh(cubeMeshIndex), scene.getShader(basicShaderIndex), Transform(glm::vec3(-2.0f, 0.0f, -1.5f))));

        std::shared_ptr<Material> sphereMaterial = std::make_shared<Material>();
        sphereMaterial->name = "PBR_Material";
        sphereMaterial->albedo = glm::vec3(0.75f, 0.75f, 0.75f);
        sphereMaterial->metallic = 0.9f;
        sphereMaterial->roughness = 0.05f;
        sphereMaterial->ao = 1.0f;

        std::shared_ptr<Material> basicMaterial = std::make_shared<Material>();
        basicMaterial->name = "Basic_Material";
        basicMaterial->albedo = glm::vec3(0.75f, 0.75f, 0.75f);
        basicMaterial->metallic = 0.9f;
        basicMaterial->roughness = 0.05f;
        basicMaterial->ao = 1.0f;
        
        //Terain generation
        NoiseGenerator terrainNoise;
        std::shared_ptr<Texture> terrainTexture = std::make_shared<Texture>("assets/textures/terrain.png");
        std::shared_ptr<Texture> sandTexture  = terrainTexture;
        std::shared_ptr<Texture> grassTexture = terrainTexture;
        std::shared_ptr<Texture> rockTexture  = std::make_shared<Texture>("assets/textures/rock.jpg");
        std::shared_ptr<Texture> snowTexture  = std::make_shared<Texture>("assets/textures/snow.jpg");
        Mesh terrainMesh = TerrainMeshGenerator::generatePatch(
            terrainNoise,
            glm::vec2(-200.0f, -200.0f),
            400.0f,
            128,
            std::vector<std::shared_ptr<Texture>>{ terrainTexture });
        std::size_t terrainMeshIndex = scene.addMesh(std::make_shared<Mesh>(std::move(terrainMesh)));
        auto terrainMaterial = std::make_shared<TerrainMaterial>();
        terrainMaterial->name = "Terrain_Material";
        terrainMaterial->albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        terrainMaterial->metallic = 0.0f;
        terrainMaterial->roughness = 0.9f;
        terrainMaterial->sandTexture = sandTexture;
        terrainMaterial->grassTexture = grassTexture;
        terrainMaterial->rockTexture = rockTexture;
        terrainMaterial->snowTexture = snowTexture;

        scene.addWorldObject(WorldObject(
                            scene.getMesh(terrainMeshIndex),
                            scene.getShader(terrainShaderIndex),
                            "Terrain",
                            Transform(glm::vec3(0.0f, -5.0f, 0.0f)),
                            terrainMaterial,
                            WorldObject::SuperType::Terrain));



        KeyInput keyInput(std::vector<int>{GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_Z, GLFW_KEY_X, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE});
        KeyInput::setupKeyInputs(window);

        std::shared_ptr<Camera> camera = std::make_shared<Camera>(60.0f, aspectRatio, 0.1f, 50000.0f, glm::vec3(0.0f, 1.5f, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), keyInput);
        scene.setCamera(camera);
        
        TerrainEditorPanel terrainEditor(scene.getMesh(terrainMeshIndex), glm::vec2(-100.0f, -100.0f), 200.0f, 128);
        EditorGUI editorGUI(window.handle(), &terrainEditor);
        
        //HDRTexture hdrPanorama("assets/hdri/studio.hdr");
        HDRTexture hdrPanorama("assets/hdri/EveningSkyHDRI046B_4K_TONEMAPPED.jpg");
        std::unique_ptr<Cubemap> envCubemap     = IBLGenerator::equirectangularToCubemap(hdrPanorama, 512);
        std::unique_ptr<Cubemap> irradianceMap  = IBLGenerator::convolveIrradiance(*envCubemap, 32);
        std::unique_ptr<Cubemap> prefilterMap   = IBLGenerator::prefilterEnvironment(*envCubemap, 128, 5);
        Texture brdfLUT                         = IBLGenerator::generateBRDFLUT(512);

        scene.getShader(pbrShaderIndex)->bind();
        scene.getShader(pbrShaderIndex)->setInt("uIrradianceMap", 5);
        scene.getShader(pbrShaderIndex)->setInt("uPrefilterMap", 6);
        scene.getShader(pbrShaderIndex)->setInt("uBRDFLUT", 7);

        scene.getShader(terrainShaderIndex)->bind();
        scene.getShader(terrainShaderIndex)->setInt("uIrradianceMap", 5);
        scene.getShader(terrainShaderIndex)->setInt("uPrefilterMap", 6);
        scene.getShader(terrainShaderIndex)->setInt("uBRDFLUT", 7);

        auto skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
        Mesh skyboxCube   = Mesh::createColoredCube();
        
        const glm::vec3 clearColor(0.05f, 0.07f, 0.10f);


        while (window.isOpen())
        {
            float time = static_cast<float>(glfwGetTime());

            editorGUI.beginFrame();
            editorGUI.draw(scene);

            glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDepthFunc(GL_LEQUAL);
            skyboxShader->bind();
            skyboxShader->setMat4("uView", camera->getViewMatrix());
            skyboxShader->setMat4("uProjection", camera->getProjectionMatrix());
            envCubemap->bind(0);
            skyboxShader->setInt("uEnvironmentMap", 0);
            skyboxCube.draw();
            glDepthFunc(GL_LESS); 

            irradianceMap->bind(5);
            prefilterMap->bind(6);
            brdfLUT.bind(7);

            //scene.getWorldObject(cube1Index).transform().setRotation(glm::vec3( time * 2.0f,  time * 50.0f,  time * 10.0f));
            //scene.getWorldObject(cube2Index).transform().setRotation(glm::vec3(-time * 2.0f, -time * 50.0f, -time * 10.0f));
            //scene.getWorldObject(sphereIndex).transform().setRotation(glm::vec3(0.0f        ,  time * 50.0f,          0.0f));


            camera->update();
            

            scene.RenderScene();
            editorGUI.render();
            window.swapBuffersAndPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr <<"\e[1m" <<  "Fatal error: " << "\e[0m" << e.what() << std::endl;
        return -1;
    }

    return 0;
}
