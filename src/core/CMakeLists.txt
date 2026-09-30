add_library(pbr_core OBJECT
    Camera.cpp
    KeyInput.cpp
    Window.cpp
    ObjectLoader.cpp
    MouseInput.cpp
)

target_include_directories(pbr_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs/glad/include
)

target_link_libraries(pbr_core
    PUBLIC
        glfw
        glad
        glm::glm
        tinyobjloader
)

# Keep the core build independent from the renderer and app code.
# If you later add platform-specific compile defs, put them here.
