cmake_minimum_required(VERSION 3.20)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)

project(PBRRenderer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
option(PBR_ENABLE_ASAN "Enable AddressSanitizer (Debug)" OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(PBR_ENABLE_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address -g)
    add_link_options(-fsanitize=address)
endif()

# ---------------------------------------------------------------------------
# Dependencies - fetched automatically via FetchContent
# ---------------------------------------------------------------------------
include(FetchContent)

# --- GLFW (window + context) ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# --- GLM (math library) ---
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

# --- Assimp (model loader) ---
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
# Limit to the formats you actually need - speeds up the build
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.3
    GIT_SHALLOW    TRUE
)

# --- tinyobjloader (OBJ model loading) ---
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        v2.0.0rc10
    GIT_SHALLOW    TRUE
)

# --- Dear ImGui (control panel, docking branch) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glm assimp tinyobjloader imgui)

# ImGui has no CMakeLists of its own for the backends - build it as a static library
FetchContent_GetProperties(imgui)
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

# --- GLAD (OpenGL loader) ---
# GLAD is not a CMake package - generate the files at https://glad.dav1d.de
# (Language: C/C++, API gl 4.5+, Profile: Core) and place them into libs/glad/{include,src}.
# If the directory does not exist, fail with a clear message instead of an obscure build error.
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/libs/glad)
if(NOT EXISTS ${GLAD_DIR}/src/glad.c)
    message(FATAL_ERROR
        "Missing generated GLAD files in ${GLAD_DIR}.\n"
        "Generate the loader at https://glad.dav1d.de (C/C++, gl 4.5, Core, "
        "no extra extensions needed) and unpack it so that the following exist:\n"
        "  libs/glad/include/glad/glad.h\n"
        "  libs/glad/include/KHR/khrplatform.h\n"
        "  libs/glad/src/glad.c"
    )
endif()
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# --- stb_image (header-only, just add the include dir) ---
# Download stb_image.h into libs/stb/stb_image.h:
# https://github.com/nothings/stb/blob/master/stb_image.h
set(STB_DIR ${CMAKE_SOURCE_DIR}/libs/stb)
if(NOT EXISTS ${STB_DIR}/stb_image.h)
    message(FATAL_ERROR
        "Missing stb_image.h in ${STB_DIR}. Download it from "
        "https://github.com/nothings/stb/blob/master/stb_image.h"
    )
endif()
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${STB_DIR})

# ---------------------------------------------------------------------------
# Sub-projects
# ---------------------------------------------------------------------------
add_subdirectory(${CMAKE_SOURCE_DIR}/src/core)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/renderer)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/editor)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/terrain)

# ---------------------------------------------------------------------------
# Main target
# ---------------------------------------------------------------------------
add_executable(PBRRenderer
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    $<TARGET_OBJECTS:pbr_core>
    $<TARGET_OBJECTS:pbr_renderer>
    $<TARGET_OBJECTS:Editor>
    $<TARGET_OBJECTS:PBR_Terrain>
)

target_include_directories(PBRRenderer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(PBRRenderer PRIVATE
    glfw
    glad
    glm::glm
    assimp
    tinyobjloader
    imgui
    stb_image
)

if(WIN32)
    target_link_libraries(PBRRenderer PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(PBRRenderer PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(PBRRenderer PRIVATE OpenGL::GL)
endif()

# ---------------------------------------------------------------------------
# Copy assets next to the binary (so relative paths work from any IDE)
# ---------------------------------------------------------------------------
add_custom_command(TARGET PBRRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:PBRRenderer>/assets
)

message(STATUS "PBRRenderer configured: build type = ${CMAKE_BUILD_TYPE}")
