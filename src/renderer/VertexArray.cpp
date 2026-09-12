#include "VertexArray.h"

#include <stdexcept>

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    release();
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : m_id(other.m_id)
    , m_nextAttributeIndex(other.m_nextAttributeIndex)
    , m_vertexBuffers(std::move(other.m_vertexBuffers))
    , m_indexBuffer(std::move(other.m_indexBuffer))
{
    other.m_id = 0;
    other.m_nextAttributeIndex = 0;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other)
    {
        release();
        m_id = other.m_id;
        m_nextAttributeIndex = other.m_nextAttributeIndex;
        m_vertexBuffers = std::move(other.m_vertexBuffers);
        m_indexBuffer = std::move(other.m_indexBuffer);
        other.m_id = 0;
        other.m_nextAttributeIndex = 0;
    }
    return *this;
}

void VertexArray::release()
{
    if (m_id != 0)
    {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
    }
}

void VertexArray::addVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
    if (vertexBuffer->layout().elements().empty())
    {
        throw std::runtime_error("VertexArray::addVertexBuffer: buffer has no layout set "
                                  "(call setLayout() before adding it)");
    }

    glBindVertexArray(m_id);
    vertexBuffer->bind();

    const auto& layout = vertexBuffer->layout();
    const auto& elements = layout.elements();
    auto elementOffsets = layout.offsets();

    for (size_t i = 0; i < elements.size(); ++i)
    {
        const auto& element = elements[i];

        glEnableVertexAttribArray(m_nextAttributeIndex);
        glVertexAttribPointer(
            m_nextAttributeIndex,
            static_cast<GLint>(element.componentCount),
            element.type,
            element.normalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(layout.stride()),
            reinterpret_cast<const void*>(static_cast<uintptr_t>(elementOffsets[i])));

        ++m_nextAttributeIndex;
    }

    glBindVertexArray(0);
    vertexBuffer->unbind();

    m_vertexBuffers.push_back(std::move(vertexBuffer));
}

void VertexArray::setIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
    glBindVertexArray(m_id);
    indexBuffer->bind();
    glBindVertexArray(0);
    // Intentionally not calling indexBuffer->unbind() here: unbinding
    // GL_ELEMENT_ARRAY_BUFFER while a VAO is bound would remove it from
    // the VAO's captured state. It is safe to unbind now since the VAO
    // is already unbound above.

    m_indexBuffer = std::move(indexBuffer);
}

void VertexArray::bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}
