#pragma once

#include <memory>

#include "../terrain/NoiseGenerator.h"
#include "../renderer/Mesh.h"

// Self-contained ImGui panel for live-tweaking terrain noise parameters.
// Owns the NoiseGenerator and its Settings; regenerates the terrain's
// Mesh in place (via move-assignment) whenever the user changes a value
// or presses "Regenerate", so the WorldObject holding this mesh needs no
// changes -- it just keeps drawing whatever the shared_ptr points to.
class TerrainEditorPanel
{
public:
    TerrainEditorPanel(std::shared_ptr<Mesh> terrainMesh,
                        const glm::vec2& origin,
                        float size,
                        int resolution,
                        const NoiseGenerator::Settings& initialSettings = NoiseGenerator::Settings());

    // Render the terrain editor controls in a standalone ImGui window.
    void draw();

    // Render the terrain editor controls inside another ImGui panel.
    void drawNested();

private:
    std::shared_ptr<Mesh> m_terrainMesh;
    NoiseGenerator m_noise;
    NoiseGenerator::Settings m_settings;

    glm::vec2 m_origin;
    float m_size;
    int m_resolution;
    std::unique_ptr<Texture> m_heightMapTexture;

    void regenerate();
    void refreshHeightMapTexture();
};
