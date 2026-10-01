#include "CameraEditorPanel.hpp"

#include <imgui.h>

CameraEditorPanel::CameraEditorPanel(std::shared_ptr<Camera> camera, float initialFov, float nearPlane, float farPlane)
    : m_camera(std::move(camera))
    , m_fov(initialFov)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane)
{
}

void CameraEditorPanel::drawNested()
{
    ImGui::Begin("Camera");

    bool projectionChanged = false;
    projectionChanged |= ImGui::SliderFloat("Field of view", &m_fov, 30.0f, 120.0f);
    projectionChanged |= ImGui::SliderFloat("Near plane", &m_nearPlane, 0.01f, 10.0f);
    projectionChanged |= ImGui::SliderFloat("Far plane", &m_farPlane, 100.0f, 100000.0f);

    if (projectionChanged)
    {
        m_camera->setProjection(m_fov, m_camera->aspectRatio(), m_nearPlane, m_farPlane);
    }

    ImGui::Separator();
    ImGui::SliderFloat("Move speed", &m_camera->moveSpeed(), 0.5f, 50.0f);
    ImGui::SliderFloat("Mouse sensitivity", &m_camera->mouseSensitivity(), 0.01f, 1.0f);

    ImGui::Separator();
    ImGui::TextUnformatted("Walk mode");

    bool walkMode = m_camera->isWalkModeEnabled();
    if (ImGui::Checkbox("Enable walking (gravity + ground collision)", &walkMode))
    {
        m_camera->setWalkModeEnabled(walkMode);
    }

    if (walkMode)
    {
        ImGui::SliderFloat("Eye height", &m_camera->eyeHeight(), 0.5f, 10.0f);
        ImGui::SliderFloat("Gravity", &m_camera->gravity(), 1.0f, 60.0f);
        ImGui::SliderFloat("Jump speed", &m_camera->jumpSpeed(), 1.0f, 20.0f);
    }

    ImGui::Separator();
    const glm::vec3 pos = m_camera->getPosition();
    ImGui::Text("Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);

    ImGui::End();
}