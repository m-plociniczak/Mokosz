// TerrainLodManager.h
#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include <renderer/Mesh.h>
#include <core/Scean.hpp>

struct ChunkLodEntry
{
    std::vector<std::shared_ptr<Mesh>> lodMeshes;
    glm::vec3 boundsCenter = glm::vec3(0.0f);
    float boundsRadius = 0.0f;
    int currentLod = -1;
    std::size_t worldObjectIndex = 0;
};

class TerrainLodManager
{
public:
    void setChunks(std::vector<ChunkLodEntry> chunks) { m_chunks = std::move(chunks); }
    void update(const glm::vec3& cameraPosition, Scean& scene);

    std::vector<float> lodDistances = { 10.0f, 40.0f, 80.0f };
    float hysteresisMargin = 20.0f;

private:
    std::vector<ChunkLodEntry> m_chunks;
};