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
using vec3i = vec3<int32_t>;
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
    vec3i pos;
    vec3i vel;

    moon_t(int32_t x, int32_t y, int32_t z)
        : pos(x, y, z), vel(0, 0, 0) { }

    int32_t pot() const {
        return std::abs(pos.x) + std::abs(pos.y) + std::abs(pos.z);
    }

    int32_t kin() const {
        return std::abs(vel.x) + std::abs(vel.y) + std::abs(vel.z);
    }

    int32_t energy() const {
        return pot() * kin();
    }
};

struct jupiter_t {
    std::vector<moon_t> moons;

    int32_t all_moon_energy() const {
        int32_t accum = 0;
        for (auto&& moon : moons)
            accum += moon.energy();
        return accum;
    }

    void step(size_t i) {
        step(i, [](size_t i) { return true; });
    }

    void step(size_t i, std::function<bool(size_t)> shouldLog) {
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
                auto signof = [](int32_t v) {
                    if (v == 0) return 0;
                    if (v < 0) return -1;
                    return 1;
                };

                pos_delta.x = signof(pos_delta.x);
                pos_delta.y = signof(pos_delta.y);
                pos_delta.z = signof(pos_delta.z);

                mj.vel = mj.vel + vec3i(-1) * pos_delta;
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
            hash ^= std::hash<int32_t>{}(m.pos.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<int32_t>{}(m.pos.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<int32_t>{}(m.pos.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

            hash ^= std::hash<int32_t>{}(m.vel.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<int32_t>{}(m.vel.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<int32_t>{}(m.vel.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };

    template <> struct hash<jupiter_t> {
        size_t operator()(jupiter_t const& j) const {
            size_t hash = 0;
            for (auto&& moon : j.moons)
                hash ^= std::hash<moon_t>{}(moon)+0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };
};

namespace stdpp {
    template <typename... Ts>
    struct lcm_t { };

    template <typename M, typename... Ts>
    struct lcm_t<M, Ts...> {
        static std::common_type_t<M, Ts...> value(M value, Ts&&... args) {
            return std::lcm(value, lcm_t<Ts...>::value(std::forward<Ts>(args)...));
        }
    };

    template <typename M>
    struct lcm_t<M> {
        static M value(M val) {
            return val;
        }
    };

    template <typename... Ts>
    std::common_type_t<Ts...> lcm(Ts&&... args) {
        return lcm_t<Ts...>::value(std::forward<Ts>(args)...);
    }
}

int main() {
    jupiter_t jupiter;
    jupiter.moons.push_back(moon_t{ 1,   3, -11 });
    jupiter.moons.push_back(moon_t{ 17, -10,  -8 });
    jupiter.moons.push_back(moon_t{ -1, -15,   2 });
    jupiter.moons.push_back(moon_t{ 12,  -4,  -4 });

    // degree of liberty hasher
    struct dol_hasher {
        std::function<size_t(moon_t const&)> pos_hasher;
        std::function<size_t(moon_t const&)> vel_hasher;
        size_t lookup;
        size_t iteration_count = 0;
        bool done = false;
        
        dol_hasher(jupiter_t const &j, std::function<size_t(moon_t const&)> pos_hasher, std::function<size_t(moon_t const&)> vel_hasher) : pos_hasher(pos_hasher), vel_hasher(vel_hasher){
            lookup = operator()(j);
            iteration_count = 0; // reset
        }

        inline size_t operator() (jupiter_t const& j) {
            if (done)
                return lookup;

            size_t hash = 0;
            for (auto&& moon : j.moons)
            {
                hash ^= pos_hasher(moon) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                hash ^= vel_hasher(moon) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            
            ++iteration_count;
            done = hash == lookup;
            return hash;
        }

        inline operator bool() const {
            return done;
        }
    };

    std::hash<int32_t> coord_hasher;
    dol_hasher xh(jupiter, [&](moon_t const& moon) { return coord_hasher(moon.pos.x); }, [&](moon_t const& moon) { return coord_hasher(moon.vel.x); });
    dol_hasher yh(jupiter, [&](moon_t const& moon) { return coord_hasher(moon.pos.y); }, [&](moon_t const& moon) { return coord_hasher(moon.vel.y); });
    dol_hasher zh(jupiter, [&](moon_t const& moon) { return coord_hasher(moon.pos.z); }, [&](moon_t const& moon) { return coord_hasher(moon.vel.z); });

    bool xl = false;
    bool yl = false;
    bool zl = false;

    auto logger = [](size_t i) { return false; };
    size_t i = 0;
    while (!xh || !yh || !zh) {
        // No logging, i is thus useless
        jupiter.step(0, logger);

        xh(jupiter);
        yh(jupiter);
        zh(jupiter);

        if (!xl && xh) {
            std::cout << "X axis hit after " << xh.iteration_count << " steps " << std::endl;
            xl = true;
        }

        if (!yl && yh) {
            std::cout << "Y axis hit after " << yh.iteration_count << " steps " << std::endl;
            yl = true;
        }

        if (!zl && zh) {
            std::cout << "Z axis hit after " << zh.iteration_count << " steps " << std::endl;
            zl = true;
        }

        ++i;
        if ((i % 100000) == 0)
            std::cout << i << " steps elapsed." << std::endl;
    }

    std::cout << stdpp::lcm(xh.iteration_count, yh.iteration_count, zh.iteration_count);

    return 0;
}
