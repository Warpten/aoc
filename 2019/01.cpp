#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>

#define STEP 2

using size_t = std::size_t;

const size_t moduleMasses[] = {
    141657,
    108912,
    57953,
    130157,
    81114,
    120358,
    53075,
    82893,
    61045,
    133276,
    118918,
    83442,
    69634,
    91360,
    114473,
    129148,
    89966,
    82662,
    91680,
    136321,
    69756,
    104495,
    94736,
    115985,
    103428,
    119147,
    83952,
    129708,
    90115,
    135629,
    96715,
    125560,
    144363,
    132711,
    108987,
    82157,
    62962,
    66436,
    87388,
    57312,
    146342,
    145167,
    67944,
    133897,
    149537,
    72973,
    76650,
    129914,
    91527,
    149755,
    128253,
    132606,
    86480,
    118149,
    132469,
    81445,
    112436,
    83057,
    75936,
    55345,
    96544,
    121752,
    78590,
    72417,
    148164,
    99428,
    85137,
    132276,
    96763,
    106806,
    106563,
    81540,
    108366,
    119792,
    79118,
    62130,
    137706,
    124220,
    135189,
    72519,
    81616,
    56448,
    53525,
    86792,
    98569,
    139069,
    67337,
    101651,
    148685,
    73432,
    94946,
    122955,
    144581,
    121306,
    126622,
    130625,
    125811,
    128017,
    55621,
    132501
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