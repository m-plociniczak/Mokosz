#include "Window.h"

#include <stdexcept>
#include <iostream>

namespace
{

    bool g_glfwInitialized = false;
}

Window::Window(int width, int height, const std::string& title)
    : m_width(width)
    , m_height(height)
{
    if (!g_glfwInitialized)
    {
        if (glfwInit() == GLFW_FALSE)
        {
            throw std::runtime_error("Window: glfwInit() failed");
        }
        g_glfwInitialized = true;
    }

        // Request an OpenGL 4.5 core profile context. Core profile removes the
        // fixed-function pipeline, which is required for the DSA-style calls
        // and modern shader-only rendering used throughout this project.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        // macOS only exposes 4.1 core at most and requires this flag for
        // forward compatibility; kept here for portability even though the
        // primary target is Windows.
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif

    #ifndef NDEBUG
        // Requests a debug context so glDebugMessageCallback actually fires.
        // Has no effect in a Release build without this hint on some drivers.
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #endif

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (m_handle == nullptr)
    {
        throw std::runtime_error("Window: glfwCreateWindow() failed "
                                  "(check that your GPU driver supports OpenGL 4.5 core)");
    }

    glfwMakeContextCurrent(m_handle);

    // GLAD must be loaded only after a context is current.
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
    {
        glfwDestroyWindow(m_handle);
        throw std::runtime_error("Window: gladLoadGLLoader() failed to load OpenGL functions");
    }

    glViewport(0, 0, width, height);

    glfwSetWindowUserPointer(m_handle, this);
    glfwSetFramebufferSizeCallback(m_handle, framebufferSizeCallback);

    glfwSwapInterval(0);

    enableDebugOutputIfSupported();

    std::cout << "Window: OpenGL context created" << std::endl;
    std::cout << "  Vendor:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "  Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "  Version:  " << glGetString(GL_VERSION) << std::endl;
}

Window::~Window()
{
    if (m_handle != nullptr)
    {
        glfwDestroyWindow(m_handle);
    }

    if (g_glfwInitialized)
    {
        glfwTerminate();
        g_glfwInitialized = false;
    }
}

bool Window::isOpen() const
{
    return glfwWindowShouldClose(m_handle) == GLFW_FALSE;
}

void Window::swapBuffersAndPollEvents()
{
    glfwSwapBuffers(m_handle);
    glfwPollEvents();
}

void Window::close()
{
    glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
}

void Window::framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
{
    // Ignore zero-size events (happens transiently on minimize on some platforms).
    if (width == 0 || height == 0)
    {
        return;
    }

    glViewport(0, 0, width, height);

    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    if (self != nullptr)
    {
        self->m_width = width;
        self->m_height = height;
        if (self->m_resizeCallback)
        {
            self->m_resizeCallback(width, height);
        }
    }
}

void Window::glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        return;
    }

    const char* sourceStr = "Unknown";
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:              sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:    sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:  sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:      sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:      sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:            sourceStr = "Other"; break;
        default: break;
    }

    const char* typeStr = "Unknown";
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:                typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:  typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:   typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY:          typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:          typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:               typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_OTHER:                typeStr = "Other"; break;
        default: break;
    }

    const char* severityStr = "Unknown";
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:   severityStr = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:    severityStr = "LOW"; break;
        default: break;
    }

    std::cerr << "[GL " << severityStr << "] (" << sourceStr << " / " << typeStr
              << ", id=" << id << "): " << message << std::endl;
}

void Window::enableDebugOutputIfSupported()
{
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) == 0)
    {
        return;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}
