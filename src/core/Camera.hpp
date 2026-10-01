#pragma once

#include <glm/glm.hpp>
#include <terrain/TerrainCollider.hpp>

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

        void update(float deltaTime);

        inline const glm::mat4& getProjectionMatrix()   const { return m_projectionMatrix; }
        inline const glm::mat4& getViewMatrix()         const { return m_viewMatrix; }
        inline const glm::vec3& getPosition()           const { return m_cameraPosition; }
        inline void setPosition(const glm::vec3& position)    { m_cameraPosition = position; }

        inline void setTerrainCollider(const TerrainCollider* collider) { m_terrainCollider = collider; }
        inline void setWalkModeEnabled(bool enabled)                    { m_walkModeEnabled = enabled; m_isGrounded = false; }
        inline bool isWalkModeEnabled() const                           { return m_walkModeEnabled; }

        inline float& moveSpeed()          { return m_cameraSpeed; }
        inline float& mouseSensitivity()   { return m_mouseSensitivity; }
        inline float& gravity()            { return m_gravity; }
        inline float& jumpSpeed()          { return m_jumpSpeed; }
        inline float& eyeHeight()          { return m_eyeHeight; }

        float aspectRatio() const { return m_aspectRatio; }

    private:
        glm::mat4 m_projectionMatrix;
        glm::mat4 m_viewMatrix;

        const KeyInput& m_keyInput;
        const MouseInput& m_mouseInput;
        const TerrainCollider* m_terrainCollider = nullptr;

        glm::vec3 m_cameraPosition;
        glm::vec3 m_cameraFront;
        glm::vec3 m_cameraUp;

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
        float m_cameraSpeed = 5.0f;   
        float m_mouseSensitivity = 0.1f;   
        float m_aspectRatio = 1.0f;        

        bool m_walkModeEnabled = false;
        bool m_isGrounded = false;
        float m_verticalVelocity = 0.0f;
        float m_gravity = 20.0f;    
        float m_jumpSpeed = 8.0f;   
        float m_eyeHeight = 1.7f;   
};