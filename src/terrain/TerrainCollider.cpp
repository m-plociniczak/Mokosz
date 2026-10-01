#include "TerrainCollider.hpp"

const TerrainHeightField* TerrainCollider::findChunkContaining(float worldX, float worldZ) const
{
    for (const auto& chunk : m_chunks)
    {
        const float chunkSize = static_cast<float>(chunk.pointsPerAxis - 1) * chunk.cellSize;
        const float minX = chunk.origin.x;
        const float minZ = chunk.origin.y;
        const float maxX = minX + chunkSize;
        const float maxZ = minZ + chunkSize;

        if (worldX >= minX && worldX <= maxX && worldZ >= minZ && worldZ <= maxZ)
        {
            return &chunk;
        }
    }
    return nullptr;
}

float TerrainCollider::getHeightAt(float worldX, float worldZ, float fallbackHeight) const
{
    const TerrainHeightField* chunk = findChunkContaining(worldX, worldZ);
    if (!chunk)
    {
        return fallbackHeight;
    }
    return chunk->getHeightAt(worldX, worldZ);
}