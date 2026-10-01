#pragma once

#include <vector>
#include "TerrainHeightField.h"

class TerrainCollider
{
public:
    inline void setChunks(std::vector<TerrainHeightField> chunkHeightFields)    { m_chunks = std::move(chunkHeightFields); }
    inline void updateChunks(std::vector<TerrainHeightField> chunkHeightFields) { m_chunks = std::move(chunkHeightFields); }
    float getHeightAt(float worldX, float worldZ, float fallbackHeight = 0.0f) const;

private:
    std::vector<TerrainHeightField> m_chunks;
    const TerrainHeightField* findChunkContaining(float worldX, float worldZ) const;

};