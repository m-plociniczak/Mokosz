#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <exception>
#include <vector>

#include <core/Window.h>
#include <core/Camera.hpp>
#include <core/KeyInput.hpp>
#include <core/MouseInput.hpp>
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
#include <terrain/TerrainWorldGenerator.hpp>
#include <terrain/TerrainHeightField.h>
#include <terrain/TerrainLodManager.hpp>
#include <terrain/TerrainCollider.hpp>

#include <editor/EditorGUI.h>
#include <editor/TerrainEditorPanel.h>
#include <editor/ChunkBoundaryRenderer.hpp>
#include <editor/CameraEditorPanel.hpp>


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
        auto waterShader = std::make_shared<Shader>("assets/shaders/water.vert", "assets/shaders/water.frag");
        std::size_t waterShaderIndex = scene.addShader(waterShader);

        std::size_t cubeMeshIndex    = scene.addMesh(std::make_shared<Mesh>(Mesh::createColoredCube()));
        std::size_t sphereMeshIndex  = scene.addMesh(std::make_shared<Mesh>(Mesh::createUVSphere(1.0f, 32, 32)));

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

        // ---------------------------------------------------------------
        // Terrain generation - chunks + LOD
        // ---------------------------------------------------------------
        NoiseGenerator terrainNoise;
        std::shared_ptr<Texture> terrainTexture = std::make_shared<Texture>("assets/textures/terrain.png");
        std::shared_ptr<Texture> sandTexture  = terrainTexture;
        std::shared_ptr<Texture> grassTexture = terrainTexture;
        std::shared_ptr<Texture> rockTexture  = std::make_shared<Texture>("assets/textures/rock.jpg");
        std::shared_ptr<Texture> snowTexture  = std::make_shared<Texture>("assets/textures/snow.jpg");

        auto terrainMaterial = std::make_shared<TerrainMaterial>();
        terrainMaterial->name = "Terrain_Material";
        terrainMaterial->albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        terrainMaterial->metallic = 0.0f;
        terrainMaterial->roughness = 0.9f;
        terrainMaterial->sandTexture = sandTexture;
        terrainMaterial->grassTexture = grassTexture;
        terrainMaterial->rockTexture = rockTexture;
        terrainMaterial->snowTexture = snowTexture;

        WorldGenerationParams worldParams;
        worldParams.chunksX = 4;
        worldParams.chunksZ = 4;
        worldParams.chunkResolution = 64;   
        worldParams.chunkWorldSize = 100.0f;
        worldParams.worldOrigin = glm::vec2(-200.0f, -200.0f); 

        std::vector<int> lodStrides = { 1, 2, 4, 8 };

        TerrainWorldGenerator terrainWorldGenerator;

        std::vector<TerrainHeightField> chunkHeightFields;
        auto chunkLodMeshesRaw = terrainWorldGenerator.sliceInChunksWithLods(
            terrainNoise, worldParams, lodStrides, &chunkHeightFields,
            std::vector<std::shared_ptr<Texture>>{ terrainTexture });

        std::vector<ChunkLodEntry> chunkLodEntries;
        chunkLodEntries.reserve(chunkLodMeshesRaw.size());

        std::vector<std::shared_ptr<Mesh>> lod0MeshesForEditor;
        lod0MeshesForEditor.reserve(chunkLodMeshesRaw.size());

        for (std::size_t i = 0; i < chunkLodMeshesRaw.size(); ++i)
        {
            ChunkLodEntry entry;
            entry.lodMeshes.reserve(chunkLodMeshesRaw[i].size());

            for (auto& mesh : chunkLodMeshesRaw[i])
            {
                entry.lodMeshes.push_back(std::make_shared<Mesh>(std::move(mesh)));
            }

            const TerrainHeightField& hf = chunkHeightFields[i];
            const float chunkSize = static_cast<float>(hf.pointsPerAxis - 1) * hf.cellSize;
            entry.boundsCenter = glm::vec3(hf.origin.x + chunkSize * 0.5f, 0.0f, hf.origin.y + chunkSize * 0.5f);
            entry.boundsRadius = chunkSize * 0.7071f;

            std::size_t meshIndex = scene.addMesh(entry.lodMeshes[0]);
            std::size_t objectIndex = scene.addWorldObject(WorldObject(
                                scene.getMesh(meshIndex),
                                scene.getShader(terrainShaderIndex),
                                "Terrain_chunk_" + std::to_string(i),
                                Transform(glm::vec3(0.0f, -5.0f, 0.0f)),
                                terrainMaterial,
                                WorldObject::SuperType::Terrain));

            entry.worldObjectIndex = objectIndex;
            lod0MeshesForEditor.push_back(entry.lodMeshes[0]);
            chunkLodEntries.push_back(std::move(entry));
        }

        const std::size_t waterMeshIndex = scene.addMesh(std::make_shared<Mesh>(
            Mesh::createPlane(worldParams.chunkWorldSize * static_cast<float>(worldParams.chunksX),
                              worldParams.chunkWorldSize * static_cast<float>(worldParams.chunksZ))));
        auto waterMaterial = std::make_shared<Material>();
        waterMaterial->name = "Water_Material";
        waterMaterial->albedo = glm::vec3(0.05f, 0.25f, 0.85f);
        scene.addWorldObject(WorldObject(
            scene.getMesh(waterMeshIndex),
            scene.getShader(waterShaderIndex),
            "Water",
            Transform(glm::vec3(0.0f, 0.0f, 0.0f)),
            waterMaterial));

        TerrainLodManager terrainLodManager;
        terrainLodManager.lodDistances = { 150.0f, 400.0f, 900.0f };
        terrainLodManager.hysteresisMargin = 20.0f;
        terrainLodManager.setChunks(std::move(chunkLodEntries));

        KeyInput keyInput(std::vector<int>{
            GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
            GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE, GLFW_KEY_TAB, GLFW_KEY_F});
        KeyInput::setupKeyInputs(window);

        MouseInput mouseInput;
        MouseInput::setupMouseInput(window);
        std::shared_ptr<TerrainCollider> terrainCollider = std::make_shared<TerrainCollider>();
        terrainCollider->setChunks(chunkHeightFields);

        std::shared_ptr<Camera> camera = std::make_shared<Camera>(
            45.0f, aspectRatio, 0.1f, 50000.0f,
            glm::vec3(0.0f, 1.5f, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
            keyInput, mouseInput);
            scene.setCamera(camera);

        camera->setTerrainCollider(terrainCollider.get());
        camera->setWalkModeEnabled(true);

        const glm::vec2 previewOrigin = worldParams.worldOrigin;
        const float previewSize = worldParams.chunkWorldSize * static_cast<float>(worldParams.chunksX);
        const int previewResolution = worldParams.chunksX * worldParams.chunkResolution;

        auto debugLineShader = std::make_shared<Shader>("assets/shaders/debug_line.vert", "assets/shaders/debug_line.frag");
        auto chunkBoundaryRenderer = std::make_shared<ChunkBoundaryRenderer>(debugLineShader);
        chunkBoundaryRenderer->build(chunkHeightFields);

        TerrainEditorPanel terrainEditor(
                        terrainLodManager,
                        terrainCollider,      
                        lodStrides,
                        previewOrigin,
                        previewSize,
                        previewResolution,
                        worldParams,
                        terrainWorldGenerator,
                        chunkBoundaryRenderer,
                        std::vector<std::shared_ptr<Texture>>{ terrainTexture });; 

        EditorGUI editorGUI(window.handle(), &terrainEditor);
        CameraEditorPanel cameraEditor(camera, 60.0f, 0.1f, 50000.0f);

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

        bool cursorCaptured = true;
        bool tabWasPressed = false;
        bool fWasPressed = false;

        while (window.isOpen())
        {
            float time = static_cast<float>(glfwGetTime());

            bool tabIsPressed = keyInput.isKeyPressed(GLFW_KEY_TAB);
            if (tabIsPressed && !tabWasPressed)
            {
                cursorCaptured = !cursorCaptured;
                glfwSetInputMode(window.handle(), GLFW_CURSOR, cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            }
            tabWasPressed = tabIsPressed;

            bool fIsPressed = keyInput.isKeyPressed(GLFW_KEY_F);
            if (fIsPressed && !fWasPressed)
            {
                camera->setWalkModeEnabled(!camera->isWalkModeEnabled());
            }
            fWasPressed = fIsPressed;

            editorGUI.beginFrame();
            editorGUI.draw(scene);
            terrainEditor.draw();      
            terrainEditor.drawNested();
            cameraEditor.drawNested();
            

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

            if (cursorCaptured)
            {
                camera->update(time / 100.f);
            }

            terrainLodManager.update(camera->getPosition(), scene);

            waterShader->bind();
            waterShader->setFloat("uTime", time);
            scene.RenderScene();
            editorGUI.render();

            mouseInput.endFrame(); 
            window.swapBuffersAndPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "\e[1m" << "Fatal error: " << "\e[0m" << e.what() << std::endl;
        return -1;
    }

    return 0;
}