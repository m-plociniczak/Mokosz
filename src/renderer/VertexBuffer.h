#pragma once

#include <glad/glad.h>

#include <vector>
#include <string>
#include <cstdint>

// Describes a single vertex attribute within a buffer's layout, e.g.
// "3 floats for position" or "2 floats for a UV coordinate".
struct BufferElement
{
    std::string name;   // For debugging/logging only.
    GLenum type;         // GL_FLOAT, GL_INT, etc.
    uint32_t componentCount; // 1..4
    bool normalized;

    uint32_t size() const;
};

// An ordered list of BufferElements describing how a vertex is laid out
// in memory. Computes attribute offsets and the overall stride so
// VertexArray can wire up glVertexAttribPointer calls automatically.
class BufferLayout
{
public:
    BufferLayout() = default;
    BufferLayout(std::initializer_list<BufferElement> elements);

    uint32_t stride() const { return m_stride; }
    const std::vector<BufferElement>& elements() const { return m_elements; }

    std::vector<uint32_t> offsets() const;

private:
    std::vector<BufferElement> m_elements;
    uint32_t m_stride = 0;

    void calculateStride();
};

// RAII wrapper around an OpenGL Vertex Buffer Object (GL_ARRAY_BUFFER).
// Holds raw vertex data plus the layout describing how to interpret it;
// VertexArray reads the layout when the buffer is attached.
class VertexBuffer
{
public:
    // Creates and uploads a static vertex buffer (GL_STATIC_DRAW).
    VertexBuffer(const void* data, uint32_t sizeBytes);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void bind() const;
    void unbind() const;

    void setLayout(const BufferLayout& layout) { m_layout = layout; }
    const BufferLayout& layout() const { return m_layout; }

    GLuint id() const { return m_id; }

private:
    GLuint m_id = 0;
    BufferLayout m_layout;

    void release();
};
