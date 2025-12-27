#include "tn/LOS.h"
#include <cstdlib>

namespace tn {

    static bool IsBlocking(const GridMap& map,
        const DynamicOccupancy& occ,
        IVec2 p,
        const LOSSettings& settings
    ) 
    {
        if(!map.InBounds(p)) {
            return true;
        }

        if(settings.blockByStatic && map.IsBlockedStatic(p)) {
            return true;
        }

        if(settings.blockByOccupancy && occ.IsOccupied(p)) {
            return true;
        }

        return false;
    }

    //Bresenham
    LOSRay TraceLOS(
        const GridMap &map,
        const DynamicOccupancy &occ,
        IVec2 from,
        IVec2 to,
        const LOSSettings &settings
    )
    {
        LOSRay out;

        int x0 = from.x, y0 = from.y;
        int x1 = to.x, y1 = to.y;

        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);

        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;

        int err = dx - dy;

        auto pushCell = [&](int x, int y) {
            out.cells.push_back({x, y});
        };

        auto checkBlock = [&](int x, int y) -> bool {
            IVec2 p{x, y};

            if(!settings.includeEndpoints) {
                if (p == from || p == to) {
                    return false;
                }
            }

            if(IsBlocking(map, occ, p, settings)) {
                out.blocked = true;
                out.blockedAt = p;
                return true;
            }
            return false;
        };

        while(true) {
            pushCell(x0, y0);

            if(checkBlock(x0, y0)){
                return out;
            }

            if(x0 == x1 && y0 == y1){
                break;
            }

            int e2 = 2 * err;

            if(e2 == -dy) {

            }

            int prevX = x0;
            int prevY = y0;

            if(e2 > -dy) {
                err -= dy;
                x0 += sx;
            }

            if(e2 < dx) {
                err += dx;
                y0 += sy;
            }

            if(x0 != prevX && y0 != prevY) {
                IVec2 sideA{x0, prevY};
                IVec2 sideB{prevX, y0};

                out.cells.push_back(sideA);
                if(checkBlock(sideA.x, sideA.y)) {
                    return out;
                }

                out.cells.push_back(sideB);
                if (checkBlock(sideB.x, sideB.y))
                {
                    return out;
                }
            }
        }

        return out;
    }

    bool HasLineOfSight(
        const GridMap &map,
        const DynamicOccupancy &occ,
        IVec2 from,
        IVec2 to,
        const LOSSettings &settings
    )
    {
        LOSRay ray = TraceLOS(map, occ, from, to, settings);
        return !ray.blocked;
    }
}