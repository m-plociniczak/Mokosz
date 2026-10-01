add_library(Editor OBJECT
    EditorGUI.cpp
    TerrainEditorPanel.cpp
    ChunkBoundaryRenderer.cpp
    CameraEditorPanel.cpp
)

target_include_directories(Editor
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs/glad/include
        ${CMAKE_SOURCE_DIR}/libs/glm
        ${CMAKE_SOURCE_DIR}/libs/imgui
)

target_link_libraries(Editor
    PUBLIC
        glfw
        glad
        imgui
        glm
)