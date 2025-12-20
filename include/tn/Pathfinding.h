#pragma once
#include <vector>
#include "Types.h"
#include "GridMap.h"
#include "DynamicOccupancy.h"

namespace tn {

    struct PathSettings {
        bool allowDiagonal = true;
        bool preventCornerCuts = true;
        bool allowPartial = true;
    };

    struct PathResult {
        std::vector<IVec2> points;
        bool reachedGoal = false;
        float totalCost = 0.f;
    };

    class PathFinder {

        public:

            PathFinder(int w, int h);
            PathResult FindPath(const GridMap& map, const DynamicOccupancy& occ, IVec2 start, IVec2 goal, const PathSettings& settings);

        private:

            struct Node {
                float g = 0;
                float f = 0;
                int parent = -1;
                uint32_t opened = 0;
                uint32_t closed = 0;
            };

            int m_width;
            int m_height;
            uint32_t m_searchId = 1;
            std::vector<Node> m_nodes;
    };

}
