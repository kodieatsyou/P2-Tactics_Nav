#include "tn/TacticalEval.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace tn {

    static float Distance(IVec2 a, IVec2 b)
    {
        float dx = float(a.x - b.x);
        float dy = float(a.y - b.y);

        return std::sqrt(dx * dx + dy * dy);
    }

    static IVec2 QuantizeDirection(IVec2 from, IVec2 to) {
        int dx = to.x - from.x;
        int dy = to.y - from.y;
        dx = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
        dy = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
        return IVec2{dx, dy};
    }

    static float DirectionalCoverVsEnemy(const GridMap& map, IVec2 tile, IVec2 enemyPos) {
        IVec2 dir = QuantizeDirection(tile, enemyPos);
        if(dir.x == 0 && dir.y == 0){
            return 0.0f;
        }

        IVec2 neighbor {
            tile.x + dir.x,
            tile.y + dir.y
        };

        if(!map.InBounds(neighbor)) {
            return 0.0f;
        }

        return map.IsBlockedStatic(neighbor) ? 1.0f : 0.0f;
    }

    static float MobilityScoreLocal(const GridMap& map, const DynamicOccupancy& occ, IVec2 tile) {
        static const IVec2 dirs8[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        int open = 0;
        for(auto d : dirs8) {
            IVec2 n{
                tile.x + d.x,
                tile.y + d.y
            };
            if(!map.InBounds(n)){
                continue;
            }
            if(map.IsBlockedStatic(n)){
                continue;
            }
            if(occ.IsOccupied(n)){
                continue;
            }
            open++;
        }
        return float(open) / 8.0f;
    }

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
        const TacticalSettings &settings)
    {
        TacticalResult out;
        const int W = map.Width();
        const int H = map.Height();
        const int N = W * H;

        if((int) reachable.reachable.size() != N) {
            return out;
        }
        if((int) reachable.costTo.size() != N) {
            return out;
        }
        if(!influence.threat.empty() && (int)influence.threat.size() != N) {
            return out;
        }

        float maxObjDist = std::sqrt(float(W*W + H*H));
        if(maxObjDist < 1e-3f){
            maxObjDist = 1.0f;
        }

        float maxMoveCost = reachable.maxCostInSet > 1e-3f ? reachable.maxCostInSet : 1.0f;

        std::vector<ScoredTile> scored;
        scored.reserve(256);

        for(int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x)
            {
                IVec2 p{x, y};
                int id = y * W + x;

                if(!reachable.reachable[id]) {
                    continue;
                }
                if(map.IsBlockedStatic(p)) {
                    continue;
                }
                if(occ.IsOccupied(p)) {
                    continue;
                }

                EvalBreakdown b{};

                if(!influence.threat.empty() && influence.maxThreat > 1e-6f) {
                    b.threat = influence.threat[id] / influence.maxThreat;
                } else {
                    b.threat = 0.0f;
                }

                float bestCover = 0.0f;
                if(settings.useDirectionalCover && !enemies.empty()) {
                    for(const auto& e : enemies) {
                        float c = DirectionalCoverVsEnemy(map, p, e.pos);
                        bestCover = std::max(bestCover, c);
                    }
                }
                b.cover = bestCover;

                float attack = 0.0f;
                for(const auto& e : enemies) {
                    float d = Distance(p, e.pos);
                    if(d <= float(selfSpecs.weaponRange)) {
                        if(HasLineOfSight(map, occ, p, e.pos, settings.losAttack)) {
                            float rangeFactor = 1.0f - (d / float(selfSpecs.weaponRange));
                            attack += rangeFactor * e.weight;
                        }
                    }
                }
                if(!enemies.empty()) {
                    attack /= float(enemies.size());
                }
                b.attack = attack;

                float dObj = Distance(p, objective);
                float obj01 = 1.0f - (dObj / maxObjDist);
                if(obj01 < 0.0f) {
                    obj01 = 0.0f;
                }
                if(obj01 > 1.0f) {
                    obj01 = 1.0f;
                }
                b.objective = settings.objectiveIsCloserIsBetter ? obj01 : (1.0f - obj01);

                float cMove = reachable.costTo[id];
                float move01 = 1.0f - (cMove / maxMoveCost);
                if (move01 < 0.0f)
                {
                    move01 = 0.0f;
                }
                if (move01 > 1.0f)
                {
                    move01 = 1.0f;
                }
                b.moveCost = move01;

                b.mobility = MobilityScoreLocal(map, occ, p);

                b.total =
                    weights.w_cover * b.cover +
                    weights.w_attack * b.attack +
                    weights.w_objective * b.objective +
                    weights.w_moveCost * b.moveCost +
                    weights.w_mobility * b.mobility +
                    weights.w_threat * (-b.threat);

                scored.push_back({p, b});
            }
        }

        if(scored.empty()) {
            return out;
        }

        std::sort(scored.begin(), scored.end(), [](const ScoredTile& a, const ScoredTile& b) {
            return a.score.total > b.score.total;
        });

        out.hasResult = true;
        out.best = scored.front();

        int n = std::min(settings.topN, (int)scored.size());
        out.top.assign(scored.begin(), scored.begin() + n);
        return out;
    }
}