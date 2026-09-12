#pragma once

#include <glad/glad.h>
#include <cstdint>

class IndexBuffer
{
public:

    IndexBuffer(const uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    void bind() const;
    void unbind() const;

    uint32_t count() const { return m_count; }
    GLuint id() const { return m_id; }

private:
    GLuint m_id = 0;
    uint32_t m_count = 0;

    void release();
};
