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