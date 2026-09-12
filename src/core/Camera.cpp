#include <glm/gtc/matrix_transform.hpp>
#include "Camera.hpp"


Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane, glm::vec3 position, glm::vec3 target, glm::vec3 up, const KeyInput& keyInput)
    : m_keyInput(keyInput), m_cameraPosition(position), m_cameraFront(glm::normalize(target - position)), m_cameraUp(up)
{
    m_projectionMatrix      = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    m_viewMatrix            = glm::lookAt(position, target, up);
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

void Camera::update()
{
    if (m_keyInput.isKeyPressed(GLFW_KEY_Q)) m_yaw -= 1.0f;
    if (m_keyInput.isKeyPressed(GLFW_KEY_E)) m_yaw += 1.0f;
    if (m_keyInput.isKeyPressed(GLFW_KEY_Z)) m_pitch += 1.0f;
    if (m_keyInput.isKeyPressed(GLFW_KEY_X)) m_pitch -= 1.0f;


    m_cameraFront = glm::normalize(glm::vec3(
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
    ));


    glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));

    if (m_keyInput.isKeyPressed(GLFW_KEY_W))              m_cameraPosition += m_cameraFront * m_cameraSpeed;
    if (m_keyInput.isKeyPressed(GLFW_KEY_S))              m_cameraPosition -= m_cameraFront * m_cameraSpeed;
    if (m_keyInput.isKeyPressed(GLFW_KEY_A))              m_cameraPosition -= right * m_cameraSpeed;
    if (m_keyInput.isKeyPressed(GLFW_KEY_D))              m_cameraPosition += right * m_cameraSpeed;
    if (m_keyInput.isKeyPressed(GLFW_KEY_SPACE))          m_cameraPosition += m_cameraUp * m_cameraSpeed;
    if (m_keyInput.isKeyPressed(GLFW_KEY_LEFT_SHIFT))     m_cameraPosition -= m_cameraUp * m_cameraSpeed;

    setView(m_cameraPosition, m_cameraPosition + m_cameraFront, m_cameraUp);
}