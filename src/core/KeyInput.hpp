#pragma once

#include <vector>
#include <unordered_map>

#include "Window.h"


class KeyInput
{

    public:
        KeyInput(std::vector<int> keysToTrack);
        ~KeyInput();

        bool isKeyPressed(int key) const;

        inline bool getIsEnabled()           const  { return m_isEnabled; }
        inline void setIsEnabled(bool value)        { m_isEnabled = value;}

        // Must be called before any KeyInput instances will work
        static void setupKeyInputs(Window& window);


    
    private:
        std::vector<int>              m_keysToTrack;
        std::unordered_map<int, bool> m_keyStates;
        bool                          m_isEnabled;

        // The GLFW callback for key events.  Sends events to all KeyInput instances
        static void callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        // Keep a list of all KeyInput instances and notify them all of key events
        static std::vector<KeyInput*> m_instances;
    
};