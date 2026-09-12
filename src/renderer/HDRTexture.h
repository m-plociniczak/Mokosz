#pragma once

#include <glad/glad.h>
#include <string>

class HDRTexture
{
public:
    explicit HDRTexture(const std::string& path);
    ~HDRTexture();

    HDRTexture(const HDRTexture&) = delete;
    HDRTexture& operator=(const HDRTexture&) = delete;
    HDRTexture(HDRTexture&& other) noexcept;
    HDRTexture& operator=(HDRTexture&& other) noexcept;

    void bind(unsigned int unit) const;

    GLuint id()     const { return m_id; }
    int width()     const { return m_width; }
    int height()    const { return m_height; }

private:
    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;

    void release();
};
