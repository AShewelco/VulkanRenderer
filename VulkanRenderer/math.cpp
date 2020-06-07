#include "math.h"

namespace vkr::math {
    constexpr auto sign(float number) -> float {
        if (number > 0.0f) {
            return 1.0f;
        }
        else if (number < 0.0f) {
            return -1.0f;
        }
        else {
            return 0.0f;
        }
    }

    constexpr auto squareSigned(float number) -> float {
        return number * number * sign(number);
    }
}