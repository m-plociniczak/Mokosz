#include "Cubemap.h"

Cubemap::Cubemap(int size, GLenum internalFormat, int mipLevels)
    : m_size(size)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    for (int mip = 0; mip < mipLevels; ++mip)
    {
        int mipSize = size >> mip;
        mipSize = mipSize > 0 ? mipSize : 1;

        for (unsigned int face = 0; face < 6; ++face)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip,
                         static_cast<GLint>(internalFormat), mipSize, mipSize,
                         0, GL_RGBA, GL_FLOAT, nullptr);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                     mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (mipLevels > 1)
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap::~Cubemap()
{
    release();
}

Cubemap::Cubemap(Cubemap&& other) noexcept
    : m_id(other.m_id), m_size(other.m_size)
{
    other.m_id = 0;
}

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_id = other.m_id;
        m_size = other.m_size;
        other.m_id = 0;
    }
    return *this;
}

void Cubemap::release()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void Cubemap::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Cubemap::generateMipmaps() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
