#include "Framebuffer.h"

#include <stdexcept>
#include <string>

Framebuffer::Framebuffer(int width, int height)
{
    glGenFramebuffers(1, &m_fbo);
    glGenRenderbuffers(1, &m_rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    if (m_rbo != 0) glDeleteRenderbuffers(1, &m_rbo);
    if (m_fbo != 0) glDeleteFramebuffers(1, &m_fbo);
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height)
{
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::attachCubemapFace(GLuint cubemapId, unsigned int faceIndex, int mipLevel) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, cubemapId, mipLevel);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer: incomplete after attaching cubemap face (status=" +
                                  std::to_string(status) + ")");
    }
}

void Framebuffer::attachTexture2D(GLuint textureId) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer: incomplete after attaching 2D texture (status=" +
                                  std::to_string(status) + ")");
    }
}
