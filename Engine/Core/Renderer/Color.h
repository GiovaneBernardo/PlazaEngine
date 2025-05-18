#pragma once
#include "vec4.hpp"

namespace Plaza {
    struct PlColor : glm::vec4 {
    public:
        using glm::vec4::vec4;
        
        static const PlColor WHITE;
        static const PlColor BLACK;
        static const PlColor GRAY;
        static const PlColor RED;
        static const PlColor GREEN;
        static const PlColor BLUE;
        static const PlColor YELLOW;
        static const PlColor PURPLE;
    };
}
