#pragma once

#include <glad/glad.h>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <memory>
#include <vector>

// RAII wrapper around an OpenGL Vertex Array Object. Owns one or more
// VertexBuffers and an optional IndexBuffer; reading each VertexBuffer's
// BufferLayout, it wires up glVertexAttribPointer calls automatically so
// callers never write raw attribute-index/offset code by hand.
class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    // Attaches a vertex buffer, configuring vertex attributes based on its
    // layout(). Attribute locations continue from where the previous
    // buffer's attributes left off, so multiple buffers can be combined
    // (e.g. positions in one VBO, instance data in another).
    void addVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

    void setIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

    void bind() const;
    void unbind() const;

    const std::shared_ptr<IndexBuffer>& indexBuffer() const { return m_indexBuffer; }

private:
    GLuint m_id = 0;
    uint32_t m_nextAttributeIndex = 0;

    std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;

    void release();
};
