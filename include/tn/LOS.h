#pragma once
#include <vector>
#include "tn/Types.h"
#include "tn/GridMap.h"
#include "tn/DynamicOccupancy.h"

namespace tn {

    struct LOSSettings {
        bool blockByStatic = true;
        bool blockByOccupancy = false;
        bool includeEndpoints = false; //If true, start and end tiles are considered blockers
    };

    struct LOSRay {
        std::vector<IVec2> cells;
        bool blocked = false;
        IVec2 blockedAt{0,0};
    };

    LOSRay TraceLOS(const GridMap& map,
                    const DynamicOccupancy& occ,
                    IVec2 from,
                    IVec2 to,
                    const LOSSettings& settings);

    bool HasLineOfSight(const GridMap& map,
                        const DynamicOccupancy& occ,
                        IVec2 from,
                        IVec2 to,
                        const LOSSettings& settings);

}