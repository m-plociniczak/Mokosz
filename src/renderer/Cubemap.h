#pragma once

#include <glad/glad.h>


class Cubemap
{
public:
    Cubemap(int size, GLenum internalFormat = GL_RGBA16F, int mipLevels = 1);
    ~Cubemap();

    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    Cubemap(Cubemap&& other) noexcept;
    Cubemap& operator=(Cubemap&& other) noexcept;

    void bind(unsigned int unit) const;

    void generateMipmaps() const;

    GLuint id() const { return m_id; }
    int size() const { return m_size; }

private:
    GLuint m_id = 0;
    int m_size = 0;

    void release();
};
