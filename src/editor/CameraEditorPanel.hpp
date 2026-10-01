#pragma once

#include <memory>
#include <core/Camera.hpp>

class CameraEditorPanel
{
public:
    explicit CameraEditorPanel(std::shared_ptr<Camera> camera,
                                float initialFov = 60.0f,
                                float nearPlane = 0.1f,
                                float farPlane = 50000.0f);

    void drawNested();

private:
    std::shared_ptr<Camera> m_camera;

    float m_fov;
    float m_nearPlane;
    float m_farPlane;
};