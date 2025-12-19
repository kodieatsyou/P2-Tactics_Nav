#pragma once
#include <vector>
#include "Types.h"

namespace tn {

    class GridMap {

        public:
            GridMap(int w, int h);

            bool InBounds(IVec2 p) const;
            bool IsBlockedStatic(IVec2 p) const;
            float TerrainCost(IVec2 p) const;

            void SetBlocked(IVec2 p, bool blocked);
            void SetTerrainCost(IVec2 p, float cost);

            int Width() const {return m_width; }
            int Height() const { return m_height; }

        private:
            int m_width;
            int m_height;

            std::vector<uint8_t> m_blocked;
            std::vector<float> m_cost;
    };

}