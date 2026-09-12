#pragma once

#include <glad/glad.h>

class Framebuffer
{
public:
  
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void bind() const;
    static void unbind();

    void resize(int width, int height);

    void attachCubemapFace(GLuint cubemapId, unsigned int faceIndex, int mipLevel = 0) const;
    void attachTexture2D(GLuint textureId) const;

private:
    GLuint m_fbo = 0;
    GLuint m_rbo = 0;
};
