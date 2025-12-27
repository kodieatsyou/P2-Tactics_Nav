#include "tn/Reachability.h"
#include <queue>
#include <limits>
#include <cmath>

namespace tn
{

    ReachableSet ComputeReachableSet(
        const GridMap &map,
        const DynamicOccupancy &occ,
        IVec2 start,
        const ReachSettings &s)
    {
        ReachableSet out;
        const int W = map.Width();
        const int H = map.Height();
        const int N = W * H;

        const float INF = std::numeric_limits<float>::infinity();
        out.costTo.assign(N, INF);
        out.parent.assign(N, -1);
        out.reachable.assign(N, 0);
        out.maxCostInSet = 0.0f;

        auto idx = [&](IVec2 p) { return p.y * W + p.x; };

        if (!map.InBounds(start) || map.IsBlockedStatic(start) || occ.IsOccupied(start)){
            return out;
        }


        struct Item
        {
            float cost;
            int index;
        };
        struct Cmp
        {
            bool operator()(const Item &a, const Item &b) const { return a.cost > b.cost; }
        };
        std::priority_queue<Item, std::vector<Item>, Cmp> pq;

        int startIdx = idx(start);
        out.costTo[startIdx] = 0.0f;
        pq.push({0.0f, startIdx});

        const IVec2 dirs4[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        const IVec2 dirs8[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

        while (!pq.empty())
        {
            Item it = pq.top();
            pq.pop();

            if (it.cost != out.costTo[it.index]){
                continue;
            }

            if (it.cost > s.moveBudget){
                continue;
            }

            out.reachable[it.index] = 1;
            if (it.cost > out.maxCostInSet)
                out.maxCostInSet = it.cost;

            IVec2 c{it.index % W, it.index / W};

            const IVec2 *dirs = s.allowDiagonal ? dirs8 : dirs4;
            int dirCount = s.allowDiagonal ? 8 : 4;

            for (int i = 0; i < dirCount; ++i)
            {
                IVec2 n{c.x + dirs[i].x, c.y + dirs[i].y};
                if (!map.InBounds(n)){
                    continue;
                }

                if (map.IsBlockedStatic(n)){
                    continue;
                }
                if (occ.IsOccupied(n)){
                    continue;
                }

                if (s.preventCornerCut && (std::abs(dirs[i].x) + std::abs(dirs[i].y) == 2))
                {
                    IVec2 a{c.x + dirs[i].x, c.y};
                    IVec2 b{c.x, c.y + dirs[i].y};
                    if (map.IsBlockedStatic(a) || map.IsBlockedStatic(b)){
                        continue;
                    }

                }

                int ni = idx(n);

                float step = (dirs[i].x == 0 || dirs[i].y == 0) ? 1.0f : 1.41421356f;
                float nextCost = it.cost + step * map.TerrainCost(n);

                if (nextCost <= s.moveBudget && nextCost < out.costTo[ni])
                {
                    out.costTo[ni] = nextCost;
                    out.parent[ni] = it.index;
                    pq.push({nextCost, ni});
                }
            }
        }

        return out;
    }

}
