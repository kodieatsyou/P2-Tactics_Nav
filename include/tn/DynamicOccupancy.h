#pragma once
#include <vector>
#include "Types.h"

namespace tn {

    class DynamicOccupancy {
        
        public:
            DynamicOccupancy(int w, int h);
            
            void SetOccupied(IVec2 p, bool occupied);
            bool IsOccupied(IVec2 p) const;

        private:
            int m_width;
            int m_height;
            std::vector<uint8_t> m_occupied;
    };

}