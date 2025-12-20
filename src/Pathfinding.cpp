#include "tn/Pathfinding.h"
#include <queue>
#include <cmath>
#include <limits>
#include <algorithm>

namespace tn {

    static float CalcHeuristic(IVec2 a, IVec2 b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    PathFinder::PathFinder(int w, int h): m_width(w), m_height(h), m_nodes(w*h){};

    PathResult PathFinder::FindPath(const GridMap& map, const DynamicOccupancy& occ, IVec2 start, IVec2 goal, const PathSettings& settings) {
        PathResult result;
        m_searchId++;

        auto idx = [&](IVec2 p){ return p.y * m_width + p.x; };

        struct OpenItem {
            int index;
            float f;
        };

        auto cmp = [](const OpenItem& a, const OpenItem& b) {
            return a.f > b.f;
        };

        std::priority_queue<OpenItem, std::vector<OpenItem>, decltype(cmp)> open(cmp);

        int startIdx = idx(start);
        int goalIdx = idx(goal);

        Node& s = m_nodes[startIdx];

        s.g = 0;
        s.f = CalcHeuristic(start, goal);
        s.parent = -1;
        s.opened = m_searchId;

        open.push({startIdx, s.f});

        int bestIdx = startIdx;
        float bestH = s.f;

        const IVec2 dirs4[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        const IVec2 dirs8[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

        while(!open.empty()) {
            int curIdx = open.top().index;
            open.pop();

            Node& cur = m_nodes[curIdx];
            if(cur.closed == m_searchId) continue;
            cur.closed = m_searchId;

            IVec2 c{curIdx % m_width, curIdx / m_width};

            float h = CalcHeuristic(c, goal);
            if(h < bestH) {
                bestH = h;
                bestIdx = curIdx;
            }

            if(curIdx == goalIdx) {
                result.reachedGoal = true;
                bestIdx = curIdx;
                break;  
            }

            const IVec2* dirs = settings.allowDiagonal ? dirs8 : dirs4;
            int dirCount = settings.allowDiagonal ? 8 : 4;

            for(int i = 0; i < dirCount; ++i) {
                IVec2 n{c.x + dirs[i].x, c.y + dirs[i].y};
                if(!map.InBounds(n)) continue;
                if(map.IsBlockedStatic(n)) continue;
                if(occ.IsOccupied(n)) continue;

                if(settings.preventCornerCuts && std::abs(dirs[i].x) + std::abs(dirs[i].y) == 2) {
                    IVec2 a{c.x + dirs[i].x, c.y};
                    IVec2 b{c.x, c.y + dirs[i].y};
                    if (map.IsBlockedStatic(a) || map.IsBlockedStatic(b))
                        continue;
                }

                int ni = idx(n);
                Node& nn = m_nodes[ni];

                float step = (dirs[i].x == 0 || dirs[i].y == 0) ? 1.0f : 1.41421356f;
                float g2 = cur.g + step * map.TerrainCost(n);

                if(nn.opened != m_searchId || g2 < nn.g) {
                    nn.g = g2;
                    nn.f = g2 + CalcHeuristic(n, goal);
                    nn.parent = curIdx;
                    nn.opened = m_searchId;
                    open.push({ni, nn.f});
                }
            }
        }

        if (!result.reachedGoal && !settings.allowPartial)
        {
            return result;
        }

        int cur = bestIdx;
        while (cur != -1)
        {
            result.points.push_back({cur % m_width, cur / m_width});
            cur = m_nodes[cur].parent;
        }

        std::reverse(result.points.begin(), result.points.end());
        result.totalCost = m_nodes[bestIdx].g;
        return result;

    }

}