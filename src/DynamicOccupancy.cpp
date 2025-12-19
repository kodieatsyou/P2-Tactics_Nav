#include "tn/DynamicOccupancy.h"

namespace tn {

    DynamicOccupancy::DynamicOccupancy(int w, int h): m_width(w), m_height(h), m_occupied(w * h, 0){}

    void DynamicOccupancy::SetOccupied(IVec2 p, bool occupied) {
        m_occupied[p.y * m_width + p.x] = occupied ? 1 : 0;
    }

    bool DynamicOccupancy::IsOccupied(IVec2 p) const {
        return m_occupied[p.y * m_width + p.x];
    }

}