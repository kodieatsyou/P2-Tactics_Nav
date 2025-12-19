#pragma once
#include <cstdint>

namespace tn {

    struct IVec2 {
        int x = 0;
        int y = 0;
    };

    inline bool operator==(const IVec2& a, const IVec2& b) {
        return a.x == b.x && a.y == b.y;
    }

    inline int ToIndex(IVec2 p, int width) {
        return p.y * width + p.x;
    }

}