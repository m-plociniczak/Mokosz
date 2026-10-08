#pragma once

#include "TerrainPipelineStage.h"


class NomralizeHeightMap : public TerrainPipelineStage
{
   
    public:
        NomralizeHeightMap() = default;
        ~NomralizeHeightMap() = default;
        void apply(TerrainHeightField& heightField) override;
        
};


