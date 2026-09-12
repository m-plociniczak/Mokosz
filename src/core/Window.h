#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <functional>

// RAII wrapper around a GLFW window with an OpenGL 4.5 core context.
// Construction performs, in order: GLFW init, window creation, context
// creation, making the context current, and GLAD function loading.
// Throws std::runtime_error if any step fails.
class Window
{
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // True until the user closes the window (or Close() is called).
    bool isOpen() const;

    // Swaps buffers and polls input events. Call once per frame, after rendering.
    void swapBuffersAndPollEvents();

    // Requests the window be closed on the next isOpen() check.
    void close();

    int width() const { return m_width; }
    int height() const { return m_height; }
    GLFWwindow* handle() const { return m_handle; }

    // Called automatically on framebuffer resize; updates the GL viewport
    // and cached width/height. Exposed so Application can hook additional
    // logic (e.g. updating a camera's aspect ratio) via setResizeCallback.
    using ResizeCallback = std::function<void(int, int)>;
    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = std::move(callback); }

private:
    GLFWwindow* m_handle = nullptr;
    int m_width;
    int m_height;
    ResizeCallback m_resizeCallback;

    static void framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
    static void glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                 GLsizei length, const GLchar* message, const void* userParam);

    void enableDebugOutputIfSupported();
};
