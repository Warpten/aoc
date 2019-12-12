#include <cstdint>
#include <cmath>
#include <iostream>
#include <functional>
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
#include <chrono>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

using size_t = std::size_t;

template <typename T>
struct vec3 {
    T x;
    T y;
    T z;

    vec3(T v) : vec3(v, v, v) { }

    vec3(T x, T y, T z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    vec3 scale(T s) {
        return { x * s, y * s, z * s };
    }

    vec3 norm() const {
        if (lengthSquared() == 0)
            return *this;

        return { x / length(), y / length(), z / length() };
    }

    vec3 round(size_t precision) const {
        auto p = std::pow(10, precision);
        return { T(int64_t(x * p) / p), T(int64_t(y * p) / p), T(int64_t(z * p) / p) };
    }

    T length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    T lengthSquared() const {
        return x * x + y * y + z * z;
    }

    bool operator == (vec3 const& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
};

template <typename T>
vec3<T> operator - (vec3<T> const& l, vec3<T> const& r) {
    vec3<T> c(l);
    c.x -= r.x;
    c.y -= r.y;
    c.z -= r.z;
    return c;
}

template <typename T>
vec3<T> operator + (vec3<T> const& l, vec3<T> const& r) {
    vec3<T> c(l);
    c.x += r.x;
    c.y += r.y;
    c.z += r.z;
    return c;
}

template <typename T>
vec3<T> operator * (vec3<T> const& l, vec3<T> const& r) {
    vec3<T> c(l);
    c.x *= r.x;
    c.y *= r.y;
    c.z *= r.z;
    return c;
}

using vec3d = vec3<double>;
using vec3f = vec3<float>;

template <typename T>
std::ostream& operator << (std::ostream& s, vec3<T> const& v) {
    s << "<x=" << std::setw(3) << std::setfill(' ') << v.x << ", y=" << std::setw(3) << std::setfill(' ') << v.y << ", z=" << std::setw(3) << std::setfill(' ') << v.z << ">";
    return s;
}

namespace std {
    template <typename T>
    struct hash<vec3<T>> {
        size_t operator()(vec3<T> const& c) const {
            return std::hash<T>()(std::round(c.x) * 100000.0f)
                ^ std::hash<T>()(std::round(c.y) * 100000.0f)
                ^ std::hash<T>()(std::round(c.z) * 100000.0f);
        }
    };
}

struct moon_t {
    vec3d pos;
    vec3d vel;

    moon_t(double x, double y, double z)
        : pos(x, y, z), vel(0, 0, 0) { }

    double pot() const {
        return std::abs(pos.x) + std::abs(pos.y) + std::abs(pos.z);
    }

    double kin() const {
        return std::abs(vel.x) + std::abs(vel.y) + std::abs(vel.z);
    }

    double energy() const {
        return pot() * kin();
    }
};

struct jupiter_t {
    std::vector<moon_t> moons;

    double all_moon_energy() const {
        double accum = 0;
        for (auto&& moon : moons)
            accum += moon.energy();
        return accum;
    }

    void step(size_t i) {
        step(i, [](size_t i) { return true; });
    }

    void step(size_t i, std::function<bool(size_t)> shouldLog) {
        // TODO: Just move moons to {dx/2,dy/2,dz/2} directly - but keep checking intermediaries somehow?
        bool log = shouldLog(i);
        if (log)
            std::cout << std::endl << "After " << i << " steps" << std::endl;
        for (size_t i = 0; i < moons.size() - 1; ++i)
        {
            auto&& mi = moons[i];
            for (size_t j = i + 1; j < moons.size(); ++j)
            {
                auto&& mj = moons[j];

                auto pos_delta = mj.pos - mi.pos;
                auto signof = [](double v) {
                    if (v == 0)return 0;
                    if (v < 0) return -1;
                    return 1;
                };

                pos_delta.x = signof(pos_delta.x);
                pos_delta.y = signof(pos_delta.y);
                pos_delta.z = signof(pos_delta.z);

                mj.vel = mj.vel + vec3d(-1) * pos_delta;
                mi.vel = mi.vel + pos_delta;
            }
        }

        for (auto&& moon : moons) {
            moon.pos = moon.pos + moon.vel;

            if (log)
                std::cout << "pos=" << moon.pos << ", vel=" << moon.vel << std::endl;
        }

        if (log)
            std::cout << "total energy: " << all_moon_energy() << std::endl;
    }
};

namespace std {
    template <> struct hash<moon_t> {
        size_t operator()(moon_t const& m) const {
            size_t hash = 0;
            hash ^= std::hash<double>{}(m.pos.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<double>{}(m.pos.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<double>{}(m.pos.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

            hash ^= std::hash<double>{}(m.vel.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<double>{}(m.vel.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<double>{}(m.vel.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };

    template <> struct hash<jupiter_t> {
        size_t operator()(jupiter_t const& j) const {
            size_t hash = 0;
            for (auto&& moon : j.moons)
                hash ^= std::hash<moon_t>{}(moon) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };
};

int main() {
    jupiter_t jupiter;
    jupiter.moons.push_back(moon_t {  1,   3, -11 });
    jupiter.moons.push_back(moon_t { 17, -10,  -8 });
    jupiter.moons.push_back(moon_t { -1, -15,   2 });
    jupiter.moons.push_back(moon_t { 12,  -4,  -4 });

    //jupiter.moons.push_back(moon_t { -8, -10,   0 });
    //jupiter.moons.push_back(moon_t {  5,   5,  10 });
    //jupiter.moons.push_back(moon_t {  2,  -7,   3 });
    //jupiter.moons.push_back(moon_t {  9,  -8,  -3 });

    std::unordered_set<size_t> state_hashes;

    size_t i = 0;
    size_t state_hash = std::hash<jupiter_t>{}(jupiter);
    while (state_hashes.count(state_hash) == 0) {
        state_hashes.insert(state_hash);
        
        jupiter.step(0, [](size_t) { return false; });
        state_hash = std::hash<jupiter_t>{}(jupiter);
        ++i;
    }

    std::cout << i;

    return 0;
}
