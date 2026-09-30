#include "TerrainLodManager.hpp"
#include <iostream>
#include <algorithm>
#include <renderer/WorldObject.h>


void TerrainLodManager::update(const glm::vec3& cameraPosition, Scean& scene)
{
    auto& objects = scene.getWorldObjects();

    for (ChunkLodEntry&  entry : m_chunks)
    {
        if(entry.lodMeshes.empty())
        {
            std::cerr << "LOD not presnet skipping" << std::endl;
            continue;
        }
        
        const float dist = glm::length(cameraPosition - entry.boundsCenter);
        int desierdLOD   = 0;

        for (int i = 0; i < lodDistances.size(); i++)
        {
            const float threshold = (i + 1 <= entry.currentLod) ? lodDistances[i] + hysteresisMargin : lodDistances[i];
            if(dist > threshold) desierdLOD = i + 1;
        }

        desierdLOD = std::min(desierdLOD, static_cast<int>(entry.lodMeshes.size()) - 1);
        if (desierdLOD != entry.currentLod)
        {
            entry.currentLod = desierdLOD;
            objects[entry.worldObjectIndex].setMesh(entry.lodMeshes[desierdLOD]);
        }
        
    }
    
}

void TerrainLodManager::updateChunkMeshes(std::size_t chunkIndex, std::vector<Mesh>& newLodMeshes, const glm::vec3& newBoundsCenter, float newBoundsRadius)
{
    if (chunkIndex >= m_chunks.size())
        return;

    auto& entry = m_chunks[chunkIndex];

    const std::size_t count = std::min(entry.lodMeshes.size(), newLodMeshes.size());
    if (count != entry.lodMeshes.size())
    {
        std::cerr << "TerrainLodManager::updateChunkMeshes: chunk " << chunkIndex
                  << " expected " << entry.lodMeshes.size() << " LOD levels but got "
                  << newLodMeshes.size() << " - updating only the first " << count
                  << " (probaly stride broken)"
                  << std::endl;
    }

    for (std::size_t lod = 0; lod < count; ++lod)
    {
        *entry.lodMeshes[lod] = std::move(newLodMeshes[lod]);
    }

    entry.boundsCenter = newBoundsCenter;
    entry.boundsRadius = newBoundsRadius;
}