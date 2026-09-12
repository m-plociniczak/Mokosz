#include "Texture.h"

#include <stb_image.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>

// NOTE: the actual stb_image implementation (STB_IMAGE_IMPLEMENTATION) is
// compiled once in StbImageImpl.cpp. This file only calls the stb_image API.

Texture::Texture(const std::string& path, bool srgb)
    : m_path(path)
{
    stbi_set_flip_vertically_on_load(true); // matches OpenGL's bottom-left origin

    unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
    if (data == nullptr)
    {
        throw std::runtime_error("Texture: failed to load image '" + path + "' (" +
                                  stbi_failure_reason() + ")");
    }

    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
    switch (m_channels)
    {
        case 1:
            internalFormat = GL_R8;
            dataFormat = GL_RED;
            break;
        case 3:
            internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
            dataFormat = GL_RGB;
            break;
        case 4:
            internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            dataFormat = GL_RGBA;
            break;
        default:
            stbi_image_free(data);
            throw std::runtime_error("Texture: unsupported channel count (" +
                                      std::to_string(m_channels) + ") in '" + path + "'");
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat),
                 m_width, m_height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    std::cout << "Texture loaded: " << path << " (" << m_width << "x" << m_height
              << ", " << m_channels << " channels)" << std::endl;
}

Texture Texture::createSolidColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    Texture texture; // uses the private default constructor
    texture.m_path = "<solid color>";
    texture.m_width = 1;
    texture.m_height = 1;
    texture.m_channels = 4;

    unsigned char pixel[4] = { r, g, b, a };

    glGenTextures(1, &texture.m_id);
    glBindTexture(GL_TEXTURE_2D, texture.m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

Texture Texture::createEmpty2D(int width, int height, GLenum internalFormat)
{
    Texture texture;
    texture.m_path = "<empty>";
    texture.m_width = width;
    texture.m_height = height;
    texture.m_channels = 2; // RG

    glGenTextures(1, &texture.m_id);
    glBindTexture(GL_TEXTURE_2D, texture.m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), width, height, 0, GL_RG, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

Texture Texture::createFromFloatData(int width, int height, const std::vector<float>& data)
{
    Texture texture;
    texture.m_path = "<normalized heightmap>";
    texture.m_width = width;
    texture.m_height = height;
    texture.m_channels = 4;

    std::vector<unsigned char> rgbaData;
    rgbaData.reserve(static_cast<std::size_t>(width) * height * 4u);

    for (float value : data)
    {
        const float clamped = std::min(1.0f, std::max(0.0f, value));
        const unsigned char gray = static_cast<unsigned char>(clamped * 255.0f);
        rgbaData.push_back(gray);
        rgbaData.push_back(gray);
        rgbaData.push_back(gray);
        rgbaData.push_back(255);
    }

    glGenTextures(1, &texture.m_id);
    glBindTexture(GL_TEXTURE_2D, texture.m_id);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 rgbaData.empty() ? nullptr : rgbaData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

Texture::~Texture()
{
    release();
}

Texture::Texture(Texture&& other) noexcept
    : m_id(other.m_id)
    , m_path(std::move(other.m_path))
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_channels(other.m_channels)
{
    other.m_id = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_id = other.m_id;
        m_path = std::move(other.m_path);
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        other.m_id = 0;
    }
    return *this;
}

void Texture::release()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void Texture::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
