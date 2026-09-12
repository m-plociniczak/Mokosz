#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <utility>

Shader::Shader(const std::string& vertexPath,
               const std::string& fragmentPath,
               const std::string& geometryPath)
    : m_vertexPath(vertexPath)
    , m_fragmentPath(fragmentPath)
    , m_geometryPath(geometryPath)
{
    if (!reload())
    {
        throw std::runtime_error("Shader: initial compilation failed for '" + vertexPath + "' / '" + fragmentPath + "'");
    }
}

Shader::~Shader()
{
    if (m_programId != 0)
    {
        glDeleteProgram(m_programId);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_programId(other.m_programId)
    , m_vertexPath(std::move(other.m_vertexPath))
    , m_fragmentPath(std::move(other.m_fragmentPath))
    , m_geometryPath(std::move(other.m_geometryPath))
    , m_uniformCache(std::move(other.m_uniformCache))
{
    other.m_programId = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        if (m_programId != 0)
        {
            glDeleteProgram(m_programId);
        }

        m_programId = other.m_programId;
        m_vertexPath = std::move(other.m_vertexPath);
        m_fragmentPath = std::move(other.m_fragmentPath);
        m_geometryPath = std::move(other.m_geometryPath);
        m_uniformCache = std::move(other.m_uniformCache);

        other.m_programId = 0;
    }
    return *this;
}

void Shader::bind() const
{
    glUseProgram(m_programId);
}

std::string Shader::readFile(const std::string& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Shader: failed to open file '" + path + "'");
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::compileStage(GLenum stage, const std::string& source, const std::string& debugPath)
{
    GLuint handle = glCreateShader(stage);
    const char* src = source.c_str();
    glShaderSource(handle, 1, &src, nullptr);
    glCompileShader(handle);

    GLint success = GL_FALSE;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint logLength = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<size_t>(logLength), '\0');
        glGetShaderInfoLog(handle, logLength, nullptr, log.data());

        glDeleteShader(handle);
        std::cerr << "Shader compilation failed (" << debugPath << "):\n" << log << std::endl;
        return 0;
    }

    return handle;
}

GLuint Shader::linkProgram(GLuint vertexStage, GLuint fragmentStage, GLuint geometryStage)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexStage);
    glAttachShader(program, fragmentStage);
    if (geometryStage != 0)
    {
        glAttachShader(program, geometryStage);
    }

    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<size_t>(logLength), '\0');
        glGetProgramInfoLog(program, logLength, nullptr, log.data());

        std::cerr << "Shader linking failed (" << m_vertexPath << " + " << m_fragmentPath << "):\n" << log << std::endl;

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

bool Shader::reload()
{
    std::string vertexSource;
    std::string fragmentSource;
    std::string geometrySource;

    try
    {
        vertexSource = readFile(m_vertexPath);
        fragmentSource = readFile(m_fragmentPath);
        if (!m_geometryPath.empty())
        {
            geometrySource = readFile(m_geometryPath);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shader::reload failed: " << e.what() << std::endl;
        return false;
    }

    GLuint vertexStage = compileStage(GL_VERTEX_SHADER, vertexSource, m_vertexPath);
    GLuint fragmentStage = compileStage(GL_FRAGMENT_SHADER, fragmentSource, m_fragmentPath);
    GLuint geometryStage = 0;
    if (!m_geometryPath.empty())
    {
        geometryStage = compileStage(GL_GEOMETRY_SHADER, geometrySource, m_geometryPath);
    }

    // Bail out cleanly if any stage failed to compile.
    bool anyStageFailed = (vertexStage == 0) || (fragmentStage == 0) || (!m_geometryPath.empty() && geometryStage == 0);
    if (anyStageFailed)
    {
        if (vertexStage != 0) glDeleteShader(vertexStage);
        if (fragmentStage != 0) glDeleteShader(fragmentStage);
        if (geometryStage != 0) glDeleteShader(geometryStage);
        return false;
    }

    GLuint newProgram = linkProgram(vertexStage, fragmentStage, geometryStage);

    // Stages are no longer needed once linked (or if linking failed).
    glDeleteShader(vertexStage);
    glDeleteShader(fragmentStage);
    if (geometryStage != 0)
    {
        glDeleteShader(geometryStage);
    }

    if (newProgram == 0)
    {
        return false;
    }

    // Swap in the new program only after a successful link, so a failed
    // reload leaves the previously working shader active.
    if (m_programId != 0)
    {
        glDeleteProgram(m_programId);
    }
    m_programId = newProgram;
    m_uniformCache.clear();

    return true;
}

GLint Shader::getUniformLocation(const std::string& name)
{
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end())
    {
        return it->second;
    }

    GLint location = glGetUniformLocation(m_programId, name.c_str());
    if (location == -1)
    {
        // Not fatal: the uniform may have been optimized out by the driver
        // if unused, but this is a useful signal during development.
        std::cerr << "Shader warning: uniform '" << name << "' not found or unused ("
                  << m_vertexPath << ")" << std::endl;
    }

    m_uniformCache[name] = location;
    return location;
}

void Shader::setBool(const std::string& name, bool value)
{
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value)
{
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value)
{
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value)
{
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value)
{
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value)
{
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}