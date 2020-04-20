#ifndef TILDAC_UTILITY_HPP_INCLUDE
#define TILDAC_UTILITY_HPP_INCLUDE

#undef max
#undef min

namespace tildac {
    template<typename T>
    T min(T a, T b) {
        return a < b ? a : b;
    }

    template<typename T>
    T max(T a, T b) {
        return a > b ? a : b;
    }

    template<typename T>
    T clamp(T v, T low, T high) {
        return min(max(v, low), high);
    }
}

#endif // !TILDAC_UTILITY_HPP_INCLUDE
