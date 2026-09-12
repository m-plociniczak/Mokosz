#include "HDRTexture.h"

#include <stb_image.h>

#include <stdexcept>
#include <utility>

HDRTexture::HDRTexture(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);

    int channels = 0;
    float* data = stbi_loadf(path.c_str(), &m_width, &m_height, &channels, 3);
    if (data == nullptr)
    {
        throw std::runtime_error("HDRTexture: failed to load '" + path + "' (" +
                                  std::string(stbi_failure_reason()) + ")");
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    // GL_RGB16F: half-float storage, enough range/precision for HDR skies
    // without the memory cost of full 32-bit floats.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
}

HDRTexture::~HDRTexture()
{
    release();
}

HDRTexture::HDRTexture(HDRTexture&& other) noexcept
    : m_id(other.m_id), m_width(other.m_width), m_height(other.m_height)
{
    other.m_id = 0;
}

HDRTexture& HDRTexture::operator=(HDRTexture&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_id = other.m_id;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_id = 0;
    }
    return *this;
}

void HDRTexture::release()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void HDRTexture::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}