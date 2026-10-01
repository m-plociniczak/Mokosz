// Camera.cpp
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.hpp"

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane,
               glm::vec3 position, glm::vec3 target, glm::vec3 up,
               const KeyInput& keyInput, const MouseInput& mouseInput)
    : m_keyInput(keyInput)
    , m_mouseInput(mouseInput)
    , m_terrainCollider(nullptr)
    , m_cameraPosition(position)
    , m_cameraFront(glm::normalize(target - position))
    , m_cameraUp(up)
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
    , m_cameraSpeed(5.0f)
    , m_mouseSensitivity(0.1f)
    , m_aspectRatio(aspectRatio)
    , m_walkModeEnabled(false)
    , m_isGrounded(false)
    , m_verticalVelocity(0.0f)
    , m_gravity(20.0f)
    , m_jumpSpeed(8.0f)
    , m_eyeHeight(1.7f)
{
    m_projectionMatrix  = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    m_viewMatrix        = glm::lookAt(position, target, up);
}
Camera::~Camera(){}

void Camera::setProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    m_projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::setView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
    m_viewMatrix = glm::lookAt(position, target, up);
}

void Camera::setProjection(const glm::mat4& projectionMatrix)
{
    m_projectionMatrix = projectionMatrix;
}
void Camera::setView(const glm::mat4& viewMatrix)
{
    m_viewMatrix = viewMatrix;
}

void Camera::update(float deltaTime)
{
    m_yaw   += m_mouseInput.deltaX() * m_mouseSensitivity;
    m_pitch += m_mouseInput.deltaY() * m_mouseSensitivity;
    m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

    m_cameraFront = glm::normalize(glm::vec3(
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
    ));

    glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));

    if (m_walkModeEnabled && m_terrainCollider != nullptr)
    {
        glm::vec3 walkFront = glm::normalize(glm::vec3(
            cos(glm::radians(m_yaw)), 0.0f, sin(glm::radians(m_yaw))));

        glm::vec3 horizontalMove(0.0f);
        if (m_keyInput.isKeyPressed(GLFW_KEY_W)) horizontalMove += walkFront;
        if (m_keyInput.isKeyPressed(GLFW_KEY_S)) horizontalMove -= walkFront;
        if (m_keyInput.isKeyPressed(GLFW_KEY_A)) horizontalMove -= right;
        if (m_keyInput.isKeyPressed(GLFW_KEY_D)) horizontalMove += right;

        if (glm::length(horizontalMove) > 0.0001f)
        {
            horizontalMove = glm::normalize(horizontalMove) * m_cameraSpeed * deltaTime;
            m_cameraPosition.x += horizontalMove.x;
            m_cameraPosition.z += horizontalMove.z;
        }

        const float groundHeight = m_terrainCollider->getHeightAt(m_cameraPosition.x, m_cameraPosition.z, m_cameraPosition.y - m_eyeHeight);
        const float feetY = m_cameraPosition.y - m_eyeHeight;

        if (feetY <= groundHeight + 0.01f && m_verticalVelocity <= 0.0f)
        {
            m_cameraPosition.y = groundHeight + m_eyeHeight;
            m_verticalVelocity = 0.0f;
            m_isGrounded = true;

            if (m_keyInput.isKeyPressed(GLFW_KEY_SPACE))
            {
                m_verticalVelocity = m_jumpSpeed;
                m_isGrounded = false;
            }
        }
        else
        {
            m_isGrounded = false;
            m_verticalVelocity -= m_gravity * deltaTime;
            m_cameraPosition.y += m_verticalVelocity * deltaTime;
        }
    }
    else
    {
        
        if (m_keyInput.isKeyPressed(GLFW_KEY_W))          m_cameraPosition += m_cameraFront * m_cameraSpeed * deltaTime;
        if (m_keyInput.isKeyPressed(GLFW_KEY_S))          m_cameraPosition -= m_cameraFront * m_cameraSpeed * deltaTime;
        if (m_keyInput.isKeyPressed(GLFW_KEY_A))          m_cameraPosition -= right * m_cameraSpeed * deltaTime;
        if (m_keyInput.isKeyPressed(GLFW_KEY_D))          m_cameraPosition += right * m_cameraSpeed * deltaTime;
        if (m_keyInput.isKeyPressed(GLFW_KEY_SPACE))      m_cameraPosition += m_cameraUp * m_cameraSpeed * deltaTime;
        if (m_keyInput.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) m_cameraPosition -= m_cameraUp * m_cameraSpeed * deltaTime;
    }

    setView(m_cameraPosition, m_cameraPosition + m_cameraFront, m_cameraUp);
}