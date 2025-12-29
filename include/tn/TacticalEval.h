#pragma once
#include <vector>
#include "tn/Types.h"
#include "tn/GridMap.h"
#include "tn/DynamicOccupancy.h"
#include "tn/Reachability.h"
#include "tn/LOS.h"
#include "tn/InfluenceField.h"

namespace tn {

    struct UnitSpecs {
        int weaponRange = 8; // In tiles
        float preferRangeMin = 0; //Optional
        float preferRangeMax = 0; // Optional
    };

    struct EvalWeights {
        float w_cover = 2.0f;
        float w_threat = 3.0f;
        float w_attack = 2.0f;
        float w_objective = 1.0f;
        float w_moveCost = 0.5f;
        float w_mobility = 0.2f;
    };

    struct EvalBreakdown {
        float cover = 0;
        float threat = 0;
        float attack = 0;
        float objective = 0;
        float moveCost = 0;
        float mobility = 0;
        float total = 0;
    };

    struct ScoredTile
    {
        IVec2 tile;
        EvalBreakdown score;
    };

    struct TacticalSettings {
        int topN = 5;

        LOSSettings losAttack { 
            .blockByStatic = true, 
            .blockByOccupancy = false, 
            .includeEndpoints = false
        };

        bool useDirectionalCover = true;

        //Treat objectiveScore as closer is better
        bool objectiveIsCloserIsBetter = true;
    };

    struct TacticalResult {
        bool hasResult = false;
        ScoredTile best{};
        std::vector<ScoredTile> top; //best to worst
    };

    TacticalResult EvaluateBestTile(
        const GridMap &map,
        const DynamicOccupancy &occ,
        IVec2 selfPos,
        const UnitSpecs &selfSpecs,
        const std::vector<Enemy> &enemies,
        IVec2 objective,
        const ReachableSet &reachable,
        const InfluenceField &influence,
        const EvalWeights &weights,
        const TacticalSettings &settings);
}