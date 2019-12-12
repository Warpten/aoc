#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>

#define STEP 2

using size_t = std::size_t;

const size_t moduleMasses[] = {
    // copy-paste input here (just add commas)
};

float getRequiredFuel(float mass) {
    if (mass <= 0.0f)
        return 0.0f;

    float fuel = std::floor(mass / 3.0f) - 2;
    fuel = std::max(fuel, 0.0f);
#if STEP == 1
    return fuel;
#elif STEP == 2
    return fuel + getRequiredFuel(fuel);
#endif
}

int main() {
    float totalFuel = 0.0f;
    for (size_t module : moduleMasses)
        totalFuel += getRequiredFuel(module);

    std::cout << std::fixed << totalFuel;
}
