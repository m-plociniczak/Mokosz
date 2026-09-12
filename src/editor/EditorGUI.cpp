#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "EditorGUI.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <renderer/WorldObject.h>
#include <renderer/Material.h>
#include <renderer/Light.h>
#include <renderer/Transform.hpp>
#include <terrain/TerrainMaterial.h>
#include <editor/TerrainEditorPanel.h>

#include <core/Scean.hpp>

#include <string>



EditorGUI::EditorGUI(GLFWwindow* window, TerrainEditorPanel* terrainEditorPanel)
    : m_terrainEditorPanel(terrainEditorPanel)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    m_initialized = true;
}

EditorGUI::~EditorGUI()
{
    if (!m_initialized)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorGUI::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void EditorGUI::draw(Scean& scene)
{
    const float currentTime = static_cast<float>(glfwGetTime());
    const float deltaTime = currentTime - m_lastFrameCounterTime;

    m_frameCount++;
    if (deltaTime >= 0.5f)
    {
        m_fps = static_cast<float>(m_frameCount) / deltaTime;
        m_frameCount = 0;
        m_lastFrameCounterTime = currentTime;
    }

    m_lastFrameTime = currentTime - m_lastFrameCounterTime + deltaTime;

    ImGui::Begin("Scene Editor");

    ImGui::Checkbox("Wireframe", &m_renderWireframe);
    glPolygonMode(GL_FRONT_AND_BACK, m_renderWireframe ? GL_LINE : GL_FILL);

    ImGui::Separator();
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::Text("Frame time: %.3f ms", m_lastFrameTime * 1000.0f);

    drawObjectList(scene);

    if (m_terrainEditorPanel)
    {
        m_terrainEditorPanel->draw();
    }

    ImGui::Separator();
    drawLightEditor(scene.getLights());

    ImGui::End();
}

void EditorGUI::drawObjectList(Scean& scene)
{
    auto& objects = scene.getWorldObjects();

    ImGui::Text("Objects");

    if (objects.empty())
    {
        m_selectedObject = -1;
        ImGui::TextDisabled("No objects");
        return;
    }

    for (int i = 0; i < static_cast<int>(objects.size()); ++i)
    {
        WorldObject& object = objects[i];

        ImGui::PushID(i);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed;

        if (i == m_selectedObject)
            flags |= ImGuiTreeNodeFlags_Selected;

        const bool isOpen = ImGui::CollapsingHeader(
            object.name().c_str(),
            flags
        );

        if (ImGui::IsItemClicked())
            m_selectedObject = i;

        if (isOpen)
        {
            ImGui::Indent();

            if (ImGui::CollapsingHeader("Transform"))
            {
                ImGui::Indent();
                drawTransformEditor(object);
                ImGui::Unindent();
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Material"))
            {
                ImGui::Indent();
                drawMaterialEditor(object);
                ImGui::Unindent();
            }

            if (object.superType() == WorldObject::SuperType::Terrain)
            {
                drawTerrainPanel(object);
            }

            ImGui::Unindent();
        }

        ImGui::PopID();
    }
}


void EditorGUI::drawTransformEditor(WorldObject& object)
{
    ImGui::Text("Transform");

    Transform& transform = object.transform();

    glm::vec3 position = transform.position();
    glm::vec3 rotation = transform.rotation();
    glm::vec3 scale = transform.scale();

    if (ImGui::DragFloat3("Position", &position.x, 0.01f))
        transform.setPosition(position);

    if (ImGui::DragFloat3("Rotation", &rotation.x, 0.5f))
        transform.setRotation(rotation);

    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 100.0f))
        transform.setScale(scale);
}

void EditorGUI::drawTerrainPanel(WorldObject& object)
{
    if (!m_terrainEditorPanel)
    {
        if (ImGui::CollapsingHeader("Terrain Panel"))
        {
            ImGui::Indent();
            ImGui::TextDisabled("Terrain editor panel unavailable.");
            ImGui::Unindent();
        }
        return;
    }

    if (ImGui::CollapsingHeader("Terrain Panel"))
    {
        ImGui::Indent();
        m_terrainEditorPanel->drawNested();
        ImGui::Unindent();
    }
}

void EditorGUI::drawMaterialEditor(WorldObject& object)
{
    std::shared_ptr<Material> material = object.material();

    if (!material)
        return;

    ImGui::Separator();
    ImGui::Text(material->name.c_str());

    if (const auto terrainMaterial = std::dynamic_pointer_cast<TerrainMaterial>(material))
    {
        ImGui::Text("Terrain Material");
        ImGui::Separator();

        auto drawTextureLabel = [](const char* label, const std::shared_ptr<Texture>& texture)
        {
            ImGui::TextDisabled("%s: %s", label, texture ? texture->path().c_str() : "None");
        };

        ImGui::PushID("SandLayer");
        if (ImGui::CollapsingHeader("Sand Layer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            drawTextureLabel("Texture", terrainMaterial->sandTexture);
            ImGui::SliderFloat("Height", &terrainMaterial->sandHeight, -50.0f, 20.0f);
            ImGui::SliderFloat("Metallic", &terrainMaterial->sandMetallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &terrainMaterial->sandRoughness, 0.01f, 1.0f);
            ImGui::Unindent();
        }
        ImGui::PopID();

        ImGui::PushID("GrassLayer");
        if (ImGui::CollapsingHeader("Grass Layer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            drawTextureLabel("Texture", terrainMaterial->grassTexture);
            ImGui::SliderFloat("Height", &terrainMaterial->grassHeight, -50.0f, 20.0f);
            ImGui::SliderFloat("Metallic", &terrainMaterial->grassMetallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &terrainMaterial->grassRoughness, 0.01f, 1.0f);
            ImGui::Unindent();
        }
        ImGui::PopID();

        ImGui::PushID("RockLayer");
        if (ImGui::CollapsingHeader("Rock Layer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            drawTextureLabel("Texture", terrainMaterial->rockTexture);
            ImGui::SliderFloat("Slope", &terrainMaterial->rockSlope, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &terrainMaterial->rockMetallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &terrainMaterial->rockRoughness, 0.01f, 1.0f);
            ImGui::Unindent();
        }
        ImGui::PopID();

        ImGui::PushID("SnowLayer");
        if (ImGui::CollapsingHeader("Snow Layer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            drawTextureLabel("Texture", terrainMaterial->snowTexture);
            ImGui::SliderFloat("Height", &terrainMaterial->snowHeight, -20.0f, 30.0f);
            ImGui::SliderFloat("Blend Width", &terrainMaterial->snowBlendWidth, 0.0f, 10.0f);
            ImGui::SliderFloat("Metallic", &terrainMaterial->snowMetallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &terrainMaterial->snowRoughness, 0.01f, 1.0f);
            ImGui::Unindent();
        }
        ImGui::PopID();

        return;
    }

    ImGui::ColorEdit3("Albedo", &material->albedo.x);

    ImGui::SliderFloat(
        "Metallic",
        &material->metallic,
        0.0f,
        1.0f
    );

    ImGui::SliderFloat(
        "Roughness",
        &material->roughness,
        0.01f,
        1.0f
    );

    ImGui::SliderFloat(
        "Ambient Occlusion",
        &material->ao,
        0.0f,
        1.0f
    );
}


void EditorGUI::drawLightEditor(std::vector<Light>& lights)
{
    if (!ImGui::CollapsingHeader(
            "Lights",
            ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (lights.empty())
    {
        ImGui::TextDisabled("No lights in scene.");
        return;
    }

    ImGui::Text("Scene lights: %zu", lights.size());

    for (std::size_t i = 0; i < lights.size(); ++i)
    {
        Light& light = lights[i];

        ImGui::PushID(static_cast<int>(i));

        std::string label = "Light " + std::to_string(i);

        if (ImGui::TreeNode(label.c_str()))
        {
            ImGui::Checkbox("Directional", &light.isDirectional);

            if (light.isDirectional)
            {
                ImGui::DragFloat3(
                    "Direction",
                    &light.direction.x,
                    0.05f,
                    -1.0f,
                    1.0f,
                    "%.2f"
                );
            }
            else
            {
                ImGui::DragFloat3(
                    "Position",
                    &light.position.x,
                    0.05f,
                    -100.0f,
                    100.0f,
                    "%.2f"
                );
            }

            ImGui::DragFloat3(
                "Intensity",
                &light.color.x,
                1.0f,
                0.0f,
                1000.0f,
                "%.1f"
            );

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}

void EditorGUI::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}