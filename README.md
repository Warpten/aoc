# aoc

All samples designed to run on godbolt/clang-trunk. Hopefully all of them will run on at least C++14.

## 01

Nothing to say.

## 02

Using `unordered_map` is useless but who knows, might save me if this rocket state machine is needed again.

## 03 (C++17)

Took me a short while because i tried `std::vector` first which does not have constant or logarithmic search time.
Also debated storing segments instead of points but problem input is small enough that that's irrelevant.

### BUG!
[bd16ffa](https://github.com/Warpten/aoc/commit/bd16ffa7d2d5813172f970593ecb7d522b67cb04) is bugged.
> If a wire visits a position on the grid multiple times, use the steps value from the first time
> it visits that position when calculating the total value of a specific intersection.

The code still netted the correct result. Bah.
Fixed in [033df38](https://github.com/Warpten/aoc/commit/033df385cb17d9f84e8b41b9a94c67901700f07). Requires C++20.
