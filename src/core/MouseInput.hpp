#pragma once

#include <vector>

#include "Window.h"

class MouseInput
{
public:
    MouseInput();
    ~MouseInput();

    inline float deltaX() const { return m_deltaX; }
    inline float deltaY() const { return m_deltaY; }

    void endFrame();

    static void setupMouseInput(Window& window);

private:
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    float   m_deltaX = 0.0f;
    float   m_deltaY = 0.0f;
    double  m_lastX = 0.0;
    double  m_lastY = 0.0;
    bool    m_firstMouse = true;

    static std::vector<MouseInput*> m_instances;
};