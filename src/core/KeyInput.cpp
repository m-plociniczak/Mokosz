#include "KeyInput.hpp"


std::vector<KeyInput*> KeyInput::m_instances;

KeyInput::KeyInput(std::vector<int> keysToTrack)
    : m_keysToTrack(keysToTrack), m_isEnabled(true)
{
    for (int key : m_keysToTrack)
    {
        m_keyStates[key] = false;
    }
    m_instances.push_back(this);
}

KeyInput::~KeyInput()
{
    auto it = std::find(m_instances.begin(), m_instances.end(), this);
    if (it != m_instances.end())
    {
        m_instances.erase(it);
    }
}

bool KeyInput::isKeyPressed(int key) const
{
    auto it = m_keyStates.find(key);
    if (it != m_keyStates.end())
    {
        return it->second;
    }
    return false;
}

void KeyInput::setupKeyInputs(Window& window)
{
    glfwSetKeyCallback(window.handle(), KeyInput::callback);
}

void KeyInput::callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    for (KeyInput* instance : m_instances)
    {
        if (instance->m_isEnabled)
        {
            auto it = instance->m_keyStates.find(key);
            if (it != instance->m_keyStates.end())
            {
                if (action == GLFW_PRESS)
                {
                    it->second = true;
                }
                else if (action == GLFW_RELEASE)
                {
                    it->second = false;
                }
            }
        }
    }
}