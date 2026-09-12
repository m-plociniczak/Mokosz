add_library(pbr_renderer OBJECT
    HDRTexture.cpp
    IBLGenerator.cpp
    StbImageImpl.cpp
    Cubemap.cpp
    Framebuffer.cpp
    Texture.cpp
    IndexBuffer.cpp
    Mesh.cpp
    Shader.cpp
    VertexArray.cpp
    VertexBuffer.cpp
    WorldObject.cpp
)

target_include_directories(pbr_renderer
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs/glad/include
)

target_link_libraries(pbr_renderer
    PUBLIC
        glfw
        glad
        glm::glm
        stb_image
)
