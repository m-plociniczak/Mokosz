#include "VertexBuffer.h"

#include <stdexcept>

uint32_t BufferElement::size() const
{
    switch (type)
    {
        case GL_FLOAT:
        case GL_INT:
        case GL_UNSIGNED_INT:
            return componentCount * 4;
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return componentCount * 1;
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return componentCount * 2;
        default:
            throw std::runtime_error("BufferElement: unsupported GL type");
    }
}

BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements)
    : m_elements(elements)
{
    calculateStride();
}

void BufferLayout::calculateStride()
{
    m_stride = 0;
    for (const auto& element : m_elements)
    {
        m_stride += element.size();
    }
}

std::vector<uint32_t> BufferLayout::offsets() const
{
    std::vector<uint32_t> result;
    result.reserve(m_elements.size());

    uint32_t offset = 0;
    for (const auto& element : m_elements)
    {
        result.push_back(offset);
        offset += element.size();
    }
    return result;
}

VertexBuffer::VertexBuffer(const void* data, uint32_t sizeBytes)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
    release();
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_id(other.m_id)
    , m_layout(std::move(other.m_layout))
{
    other.m_id = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_id = other.m_id;
        m_layout = std::move(other.m_layout);
        other.m_id = 0;
    }
    return *this;
}

void VertexBuffer::release()
{
    if (m_id != 0)
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
