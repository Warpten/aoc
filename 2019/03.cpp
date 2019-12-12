#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <algorithm>

using size_t = std::size_t;

#define STEP 2

// Central port is at 0,0
struct point {
    int32_t x;
    int32_t y;

    point(int32_t x, int32_t y) : x(x), y(y) { }


    int64_t manhattan() const {
        return std::abs(x) + std::abs(y);
    }
};


bool operator == (point const& l, point const& r) {
    return l.x == r.x && l.y == r.y;
}

namespace std
{
    template <>
    struct hash<point> {
        size_t operator()(point const& p) const {
            return std::hash<int32_t>()(p.x) ^ (std::hash<int32_t>()(p.y) << 1);
        }
    };
}

struct wire {
    std::unordered_map<point, int64_t /* step count*/> points_map;

    template <size_t N>
    wire(const char(&path)[N]) {
        point last_point(0, 0);
        int64_t step_count = 0;

        for (size_t i = 0; i < N;) {
            uint8_t dir = path[i];
            size_t amount = std::atoi(&path[i + 1]);

            switch (dir)
            {
#define MAKE_CASE(ID, COMPONENT, OP)                                    \
            case ID:                                                    \
                for (size_t ofs = 0; ofs < amount; ++ofs) {             \
                    last_point.COMPONENT += OP;                         \
                    points_map.try_emplace(last_point, ++step_count);   \
                }                                                       \
                break;
            MAKE_CASE('R', x, +1)
            MAKE_CASE('L', x, -1)
            MAKE_CASE('U', y, +1)
            MAKE_CASE('D', y, -1)
            default:
                throw std::runtime_error("unhandled cmd");
            }

            size_t charCount = 0;
            while (amount > 0) {
                amount /= 10;
                ++charCount;
            }

            i += charCount + 1 + 1;
        }
    }

    std::vector<point> intersections(wire const& other) {
        std::vector<point> results;

        for (auto&& self_point : points_map)
            if (other.points_map.find(self_point.first) != other.points_map.end())
                results.push_back(self_point.first);

        return results;
    }

    int64_t step_count_of(point const& p) {
        return points_map.find(p)->second;
    }
};

constexpr const char wire_a[] = "copy wire a here";
constexpr const char wire_b[] = "copy wire b here";

// constexpr const char wire_a[] = "R75,D30,R83,U83,L12,D49,R71,U7,L72";
// constexpr const char wire_b[] = "U62,R66,U55,R34,D71,R55,D58,R83";

int main() {
    wire a(wire_a);
    wire b(wire_b);

    int64_t manhattan = std::numeric_limits<int64_t>::max();
    auto intersections = a.intersections(b);
    for (auto&& intersection : intersections) {
#if STEP == 1
        manhattan = std::min(manhattan, intersection.manhattan());
#elif STEP == 2
        manhattan = std::min(manhattan, a.step_count_of(intersection) + b.step_count_of(intersection));
#endif
    }

    std::cout << manhattan;
    return 0;
}

