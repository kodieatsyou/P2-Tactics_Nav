#include "tn/InfluenceField.h"
#include <cmath>
#include <algorithm>

namespace tn {

    static float Distance(IVec2 a, IVec2 b) {
        float dx = float(a.x - b.x);
        float dy = float(a.y - b.y);

        return std::sqrt(dx * dx + dy * dy);
    }

    InfluenceField ComputeInfluenceField(
        const GridMap& map,
        const DynamicOccupancy& occ,
        const std::vector<Enemy>& enemies,
        const InfluenceSettings& settings
    ) 
    {
        InfluenceField out;

        const int W = map.Width();
        const int H = map.Height();
        const int N = W * H;

        out.threat.assign(N, 0.0f);
        out.maxThreat = 0.0f;

        LOSSettings los{};
        los.blockByStatic = true;
        los.blockByOccupancy = settings.blockByOccupancy;
        los.includeEndpoints = false;

        for(const Enemy& e : enemies) {
            if(!map.InBounds(e.pos)) {
                continue;
            }

            for(int y = 0; y < H; ++y) 
            {
                for (int x = 0; x < W; ++x)
                {
                    IVec2 p{x, y};
                    if(map.IsBlockedStatic(p)) {
                        continue;;
                    }

                    float d = Distance(e.pos, p);
                    if(d > settings.maxRange) {
                        continue;
                    }

                    if(settings.gateByLOS) {
                        if(!HasLineOfSight(map, occ, e.pos, p, los)) {
                            continue;
                        }
                    }

                    float t = std::max(0.0f, 1.0f - (d / settings.maxRange));
                    float value = t * settings.baseThreat * e.weight;

                    int idx = y * W + x;
                    out.threat[idx] += value;
                    out.maxThreat = std::max(out.maxThreat, out.threat[idx]);
                }
            }
        }

        return out;
    }

}