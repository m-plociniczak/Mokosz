#include <algorithm>
#include "MouseInput.hpp"


std::vector<MouseInput*> MouseInput::m_instances;

MouseInput::MouseInput()
{
    m_instances.push_back(this);
}

MouseInput::~MouseInput()
{
    auto it = std::find(m_instances.begin(), m_instances.end(), this);
    if (it != m_instances.end())
    {
        m_instances.erase(it);
    }
}

void MouseInput::endFrame()
{
    m_deltaX = 0.0f;
    m_deltaY = 0.0f;
}

void MouseInput::setupMouseInput(Window& window)
{
    glfwSetInputMode(window.handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window.handle(), MouseInput::cursorPosCallback);
}

void MouseInput::cursorPosCallback(GLFWwindow*, double xpos, double ypos)
{
    for (MouseInput* instance : m_instances)
    {
        if (instance->m_firstMouse)
        {

            instance->m_lastX = xpos;
            instance->m_lastY = ypos;
            instance->m_firstMouse = false;
        }

        instance->m_deltaX += static_cast<float>(xpos - instance->m_lastX);
        instance->m_deltaY += static_cast<float>(instance->m_lastY - ypos); 

        instance->m_lastX = xpos;
        instance->m_lastY = ypos;
    }
}