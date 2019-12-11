#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <deque>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <optional>
#include <cmath>
#include <numeric>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

using size_t = std::size_t;

struct coordinate {
    int32_t x;
    int32_t y;

    coordinate() : x(0), y(0) { }
    coordinate(int32_t x, int32_t y) : x(x), y(y) { }
};

inline bool operator == (coordinate const& lhs, coordinate const& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T>
struct vec2 {
    T x;
    T y;

    vec2(T x, T y) {
        this->x = x;
        this->y = y;
    }

    vec2 scale(T s) {
        return { x * s, y * s };
    }

    vec2 norm() const {
        if (lengthSquared() == 0)
            return *this;

        return { x / length(), y / length() };
    }

    vec2 round(size_t precision) const {
        auto p = std::pow(10, precision);
        return { T(int64_t(x * p) / p), T(int64_t(y * p) / p) };
    }

    T length() const {
        return std::sqrt(x * x + y * y);
    }

    T lengthSquared() const {
        return x * x + y * y;
    }

    bool operator == (vec2 const& o) const {
        return x == o.x && y == o.y;
    }

    bool parallel(vec2 const& o) const {
        return std::abs((x / float(o.x)) - (y / float(o.y))) < 1.0e-6f;
    }
};

vec2 zero(0.0f, 0.0f);

namespace std
{
    template <>
    struct hash<coordinate> {
        size_t operator()(coordinate const& c) const {
            return std::hash<int32_t>()(c.x) ^ std::hash<int32_t>()(c.y);
        }
    };

    template <typename T>
    struct hash<vec2<T>> {
        size_t operator()(vec2<T> const& c) const {
            return std::hash<T>()(std::round(c.x) * 100000.0f) ^ std::hash<T>()(std::round(c.y) * 100000.0f);
        }
    };

    template <>
    struct equal_to<coordinate> {
        bool operator()(coordinate const& lhs, coordinate const& rhs) const {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }
    };
}

struct map_t {
    template <int32_t N>
    map_t(char const* (&m)[N]) {
        w = strlen(m[0]);
        h = N;

        for (size_t y = 0; y < N; ++y) {
            char const* x = m[y];
            while (*x != '\0') {
                if (*x == '#') {
                    asteroids.emplace(size_t(x - m[y]), y);
                }

                ++x;
            }
        }
    }

    std::vector<coordinate> get_detected_roids(int32_t xi, int32_t yi)
    {
        std::vector<coordinate> result;

        bool on_roid = false;
        for (auto const& asteroid : asteroids)
            if (asteroid.x == xi && asteroid.y == yi)
                on_roid = true;

        if (!on_roid)
            return result;

        // Raycast
        std::unordered_map<vec2<double>, std::vector<coordinate>> vec2roid;
        for (auto&& roid : asteroids) {
            vec2<double> ray = vec2<double>(roid.x - xi, roid.y - yi);
            if (ray.x == 0 && ray.y == 0)
                continue;
            vec2roid[ray.norm().round(5)].push_back(roid);
        }

        for (auto&& kv : vec2roid)
        {
            // Keep closest roid
            int32_t distanceSquared = std::numeric_limits<int32_t>::max();
            std::sort(kv.second.begin(), kv.second.end(),
                [&xi, &yi](coordinate const& lhs, coordinate const& rhs) {
                    int32_t lhs_ray = vec2<int32_t>(lhs.x - xi, lhs.y - yi).lengthSquared();
                    int32_t rhs_ray = vec2<int32_t>(rhs.x - xi, rhs.y - yi).lengthSquared();

                    return lhs_ray < rhs_ray;
                });

            result.emplace_back(std::move(kv.second.front()));
        }

        return result;
    }

    size_t find_best_location(std::vector<coordinate>& roids, int32_t& ox, int32_t& oy) {
        ox = -1;
        oy = -1;
        roids.clear();

        size_t hit_count = 0;
        for (int32_t x = 0; x < w; ++x) {
            for (int32_t y = 0; y < h; ++y) {
                auto detected_roids = get_detected_roids(x, y);
                if (detected_roids.size() > hit_count)
                {
                    roids = std::move(detected_roids);
                    hit_count = roids.size();
                    ox = x;
                    oy = y;
                }
            }
        }

        return hit_count;
    }

    std::vector<coordinate> ima_firin_mah_lazor(int32_t ox, int32_t oy) {
        std::vector<coordinate> vaporized_roids;

        while (true) {
            auto vaporized_roids_rev = get_detected_roids(ox, oy);
            if (vaporized_roids_rev.empty()) // we got em all, chief
                break;

            // no judging ok
            auto get_angle = [&ox, &oy](coordinate const& lhs) {
                double lhs_ang = 0;

                double dx = ox - lhs.x;
                double dy = oy - lhs.y;

                if (dx > 0 && dy > 0) { // top-left corner
                    lhs_ang = M_PI * 2 - std::atan(dx / dy); // 270 + ...
                }
                else if (dx > 0 && dy < 0) { // bottom-left corner
                    lhs_ang = std::atan(dx / dy) - M_PI / 2; // ... - 90
                }
                else if (dx < 0 && dy > 0) { // top-right corner
                    lhs_ang = std::atan(-1 * dx / dy);
                }
                else if (dx < 0 && dy < 0) { // bottom-right corner
                    lhs_ang = std::atan(dx / dy) + M_PI / 2; // 90 + ...
                }
                else if (dx > 0 && dy == 0) { // left
                    lhs_ang = 3 * M_PI / 2;
                } 
                else if (dx < 0 && dy == 0) { // right
                    lhs_ang = M_PI / 2;
                }
                else if (dx == 0 && dy > 0) { // above
                    lhs_ang = 0;
                }
                else if (dx == 0 && dy < 0) { // below
                    lhs_ang = M_PI;
                }

                if (lhs_ang < 0) {
                    lhs_ang += M_PI * 2;   
                }

                return lhs_ang;
            };
            
            // mah lazor is firin up and clockwise
            std::sort(vaporized_roids_rev.begin(), vaporized_roids_rev.end(),
                [&ox, &oy, &get_angle](coordinate const& lhs, coordinate const& rhs) {
                    double lhs_ang = get_angle(lhs);
                    double rhs_ang = get_angle(rhs);

                    return lhs_ang < rhs_ang;
                });

            // Delete roids
            std::vector<coordinate>::iterator begin = vaporized_roids_rev.begin();
            std::vector<coordinate>::iterator end = vaporized_roids_rev.end();
            while (begin != end)
            {
                bool needs_vaporizing = false;
                for (auto&& vroid : asteroids) {
                    if (vroid == *begin) {
                        //std::cout << "Vaping " << begin->x << ";" << begin->y << std::endl;
                        asteroids.erase(vroid);
                        break;
                    }
                }

                ++begin;
            }

            vaporized_roids.insert(vaporized_roids.end(), vaporized_roids_rev.begin(), vaporized_roids_rev.end());
        }

        return vaporized_roids;
    }


    void dump_hit_map(std::vector<coordinate> const& hits, int32_t ox, int32_t oy, bool order = false) {
        std::unordered_set<coordinate> hitset(hits.begin(), hits.end());

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                if (x == ox && y == oy)
                    std::cout << 'o';
                else if (hitset.count({ x, y }) > 0) {
                    if (order) {
                        auto index = 0;
                        for (auto&& c : hits)
                        {
                            if (c.x == x && c.y == y)
                                break;
                            ++index;
                        }
                        if (index > 9)
                            std::cout << '+';
                        else
                            std::cout << char('0' + index);
                    }
                    else
                        std::cout << '+';
                }
                else if (asteroids.count({ x, y }) > 0)
                    std::cout << '-';
                else
                    std::cout << ' ';
            }
            std::cout << std::endl;
        }
    }

    std::unordered_set<coordinate> asteroids;
    size_t w;
    size_t h;
};

const char* map_i[] = {
    ".###..#......###..#...#",
    "#.#..#.##..###..#...#.#",
    "#.#.#.##.#..##.#.###.##",
    ".#..#...####.#.##..##..",
    "#.###.#.####.##.#######",
    "..#######..##..##.#.###",
    ".##.#...##.##.####..###",
    "....####.####.#########",
    "#.########.#...##.####.",
    ".#.#..#.#.#.#.##.###.##",
    "#..#.#..##...#..#.####.",
    ".###.#.#...###....###..",
    "###..#.###..###.#.###.#",
    "...###.##.#.##.#...#..#",
    "#......#.#.##..#...#.#.",
    "###.##.#..##...#..#.#.#",
    "###..###..##.##..##.###",
    "###.###.####....######.",
    ".###.#####.#.#.#.#####.",
    "##.#.###.###.##.##..##.",
    "##.#..#..#..#.####.#.#.",
    ".#.#.#.##.##########..#",
    "#####.##......#.#.####."
};

int main() {
    map_t map(map_i);

    std::vector<coordinate> roids;
    int32_t x, y;
    std::cout << "Part 1: " << map.find_best_location(roids, x, y) << std::endl;
    map.dump_hit_map(roids, x, y);

    auto vapes = map.ima_firin_mah_lazor(x, y);
    std::cout << "Part 2: " << vapes[199].x * 100 + vapes[199].y;


    return 0;
}
