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
#include <optional>

using size_t = std::size_t;

template <int64_t... Is>
struct rocket_t;

template <int64_t I, size_t... Is>
struct rocket_t<I, Is...>
{
    constexpr const static int64_t fuel_weight = rocket_t<I>::fuel_weight + rocket_t<Is...>::fuel_weight;

    constexpr const static int64_t recursive_fuel_weight = rocket_t<I>::recursive_fuel_weight + rocket_t<Is...>::recursive_fuel_weight;
};

template <int64_t I>
struct rocket_t<I> {
    constexpr const static int64_t fuel_weight = ((I / 3) - 2) <= 0
        ? 0
        : ((I / 3) - 2);

    constexpr const static int64_t recursive_fuel_weight = 
        fuel_weight + (fuel_weight <= 0
            ? 0
            : rocket_t<fuel_weight>::recursive_fuel_weight);
};

template <int64_t... Is>
constexpr const size_t part1 = rocket_t<Is...>::fuel_weight;

template <int64_t... Is>
constexpr const size_t part2 = rocket_t<Is...>::recursive_fuel_weight;

int main() {
    std::cout << part1</* comma-separated problem input */> << std::endl;
    std::cout << part2</* comma-separated problem input */> << std::endl;
}
