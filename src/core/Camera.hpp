#pragma once
#include <glm/glm.hpp>

#include "KeyInput.hpp"
#include "MouseInput.hpp"

class Camera
{
    public:
        Camera(float fov, float aspectRatio, float nearPlane, float farPlane,
               glm::vec3 position, glm::vec3 target, glm::vec3 up,
               const KeyInput& keyInput, const MouseInput& mouseInput);
        ~Camera();

        void setProjection(float fov, float aspectRatio, float nearPlane, float farPlane);
        void setProjection(const glm::mat4& projectionMatrix);
        void setView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
        void setView(const glm::mat4& viewMatrix);

        void update();

        inline const glm::mat4& getProjectionMatrix()   const { return m_projectionMatrix; }
        inline const glm::mat4& getViewMatrix()         const { return m_viewMatrix; }
        inline const glm::vec3& getPosition()           const { return m_cameraPosition; }

    private:
        glm::mat4 m_projectionMatrix;
        glm::mat4 m_viewMatrix;

        const KeyInput& m_keyInput;
        const MouseInput& m_mouseInput;

        glm::vec3 m_cameraPosition;
        glm::vec3 m_cameraFront;
        glm::vec3 m_cameraUp;

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
        const float m_cameraSpeed = 0.1f;
        const float m_mouseSensitivity = 0.1f; 
};