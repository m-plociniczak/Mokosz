#pragma once
#include <vector>

#include <glad/glad.h>

struct GLFWwindow;
class Scean;
class WorldObject;
class Light;
class TerrainEditorPanel;

class EditorGUI
{
public:
    explicit EditorGUI(GLFWwindow* window, TerrainEditorPanel* terrainEditorPanel = nullptr);
    ~EditorGUI();

    EditorGUI(const EditorGUI&) = delete;
    EditorGUI& operator=(const EditorGUI&) = delete;

    void beginFrame();
    void draw(Scean& scene);
    void render();

private:
    void drawObjectList(Scean& scene);
    void drawTransformEditor(WorldObject& object);
    void drawMaterialEditor(WorldObject& object);
    void drawTerrainPanel(WorldObject& object);
    void drawLightEditor(std::vector<Light>& lights);

    bool m_initialized = false;
    bool m_renderWireframe = false;
    int m_selectedObject = 0;
    float m_lastFrameTime = 0.0f;
    float m_lastFrameCounterTime = 0.0f;
    int m_frameCount = 0;
    float m_fps = 0.0f;
    TerrainEditorPanel* m_terrainEditorPanel = nullptr;
};