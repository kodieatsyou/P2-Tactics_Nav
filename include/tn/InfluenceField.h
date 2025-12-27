#pragma once
#include <vector>
#include "tn/Types.h"
#include "tn/GridMap.h"
#include "tn/DynamicOccupancy.h"
#include "tn/LOS.h"

namespace tn {

    struct InfluenceSettings {
        float maxRange = 10.0f; //Range in tiles
        float baseThreat = 1.0f; //If at distance 0
        bool gateByLOS = true;
        bool blockByOccupancy = false;
    };

    struct InfluenceField {
        std::vector<float> threat;
        float maxThreat = 0.0f;
    };

    struct Enemy {
        IVec2 pos;
        float weight = 1.0f;
    };

    InfluenceField ComputeInfluenceField(
        const GridMap& map,
        const DynamicOccupancy& occ,
        const std::vector<Enemy>& enemies,
        const InfluenceSettings& settings
    );

}