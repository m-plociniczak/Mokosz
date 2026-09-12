#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>


class Shader
{
public:

    Shader(const std::string& vertexPath,
           const std::string& fragmentPath,
           const std::string& geometryPath = "");

    ~Shader();

    Shader(const Shader&)                   = delete;
    Shader& operator=(const Shader&)        = delete;
    Shader(Shader&& other)                    noexcept;
    Shader& operator=(Shader&& other)         noexcept;

    void bind() const;

    bool reload();

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat3(const std::string& name, const glm::mat3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

    GLuint id() const { return m_programId; }

private:
    GLuint m_programId = 0;

    std::string m_vertexPath;
    std::string m_fragmentPath;
    std::string m_geometryPath;
    std::unordered_map<std::string, GLint> m_uniformCache;

    static std::string readFile(const std::string& path);
    static GLuint compileStage(GLenum stage, const std::string& source, const std::string& debugPath);
    GLuint linkProgram(GLuint vertexStage, GLuint fragmentStage, GLuint geometryStage);

    GLint getUniformLocation(const std::string& name);
};