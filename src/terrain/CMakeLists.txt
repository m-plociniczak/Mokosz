add_library(PBR_Terrain OBJECT
    NoiseGenerator.cpp
    CreateNoiseMapObject.cpp
    ApplyIslandMask.cpp
    ApplyErosion.cpp
    NomralizeHeightMap.cpp
    TerrainPipeline.cpp
    TerrainMeshGenerator.cpp
    TerrainMaterial.cpp
    ApplyAmplitude.cpp  
    TerrainWorldGenerator.cpp
)

target_include_directories(PBR_Terrain
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs/glad/include
)

target_link_libraries(PBR_Terrain
    PUBLIC
        glm::glm
)
