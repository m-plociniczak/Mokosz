#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

// RAII wrapper around a 2D OpenGL texture. Loads image data via stb_image.
class Texture
{
public:
    // Loads an image file from disk and uploads it as a GL_TEXTURE_2D.
    // srgb should be true for color/albedo textures and false for data
    // textures (normal maps, roughness, etc.) once those are introduced.
    explicit Texture(const std::string& path, bool srgb = true);

    // Creates a 1x1 solid-color texture. Used as a fallback when a mesh
    // has no diffuse texture assigned, so the shader can always sample
    // uDiffuseTexture without needing a branch.
    static Texture createSolidColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

    // Allocates an uninitialized 2D texture to be filled by rendering into
    // it (via Framebuffer::attachTexture2D), rather than loading from disk.
    // Used for the BRDF LUT: internalFormat=GL_RG16F stores just the two
    // scale/bias channels the split-sum approximation needs.
    static Texture createEmpty2D(int width, int height, GLenum internalFormat = GL_RG16F);

    // Creates a floating-point 2D texture from raw height data.
    static Texture createFromFloatData(int width, int height, const std::vector<float>& data);

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(unsigned int unit) const;

    GLuint id() const { return m_id; }
    const std::string& path() const { return m_path; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    // Used internally by createSolidColor() to skip the file-loading path.
    Texture() = default;

    GLuint m_id = 0;
    std::string m_path;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;

    void release();
};
