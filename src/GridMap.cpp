#include "tn/GridMap.h"

namespace tn {

    GridMap::GridMap(int w, int h): m_width(w), m_height(h), m_blocked(w * h, 0), m_cost(w * h, 1.0f){}

    bool GridMap::InBounds(IVec2 p) const {
        return p.x >= 0 && p.y >= 0 && p.x < m_width && p.y < m_height;
    }

    bool GridMap::IsBlockedStatic(IVec2 p) const {
        return m_blocked[p.y * m_width + p.x] != 0;
    }

    float GridMap::TerrainCost(IVec2 p) const {
        return m_cost[p.y * m_width + p.x];
    }

    void GridMap::SetBlocked(IVec2 p, bool blocked) {
        m_blocked[p.y * m_width + p.x] = blocked ? 1 : 0;
    }

    void GridMap::SetTerrainCost(IVec2 p, float cost)
    {
        m_cost[p.y * m_width + p.x] = cost;
    }
}