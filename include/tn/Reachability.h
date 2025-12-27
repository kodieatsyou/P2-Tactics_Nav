#pragma once
#include <vector>
#include "tn/Types.h"
#include "tn/GridMap.h"
#include "tn/DynamicOccupancy.h"

namespace tn
{
    struct ReachSettings
    {
        bool allowDiagonal = true;
        bool preventCornerCut = true;
        float moveBudget = 10.0f;
    };

    struct ReachableSet
    {
        std::vector<float> costTo;      // INF if unreachable
        std::vector<int> parent;
        std::vector<uint8_t> reachable;
        float maxCostInSet = 0.0f;
    };

    ReachableSet ComputeReachableSet(
        const GridMap &map,
        const DynamicOccupancy &occ,
        IVec2 start,
        const ReachSettings &settings
    );
}
