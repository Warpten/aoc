# aoc

All samples designed to run on godbolt/clang-trunk. Hopefully all of them will run on at least C++14.

## 01

Nothing to say.

## 02

Using `unordered_map` is useless but who knows, might save me if this rocket state machine is needed again.

## 03 (C++17)

Took me a short while because i tried `std::vector` first which does not have constant or logarithmic search time.
Also debated storing segments instead of points but problem input is small enough that that's irrelevant.

### :warning: BUG!
[bd16ffa](https://github.com/Warpten/aoc/commit/bd16ffa7d2d5813172f970593ecb7d522b67cb04) is bugged.
> If a wire visits a position on the grid multiple times, use the steps value from the first time
> it visits that position when calculating the total value of a specific intersection.

I completely missed that line of the problem statement. The code still netted the correct result. Bah.
Fixed in [033df38](https://github.com/Warpten/aoc/commit/033df385cb17d9f84e8b41b9a94c67901700f07). Requires C++20.

## 03_constexpr (C++17)

Will only build on Clang 9.0.0, with `--std=c++17`. Requires **insane** `-ftemplate_depth`. Should be able to solve step 1. Step 2 is another problem entirely.

## 04

This solution is horrible and there is a simpler way to attack this problem.

Consider an alphabet of 3 digits (`1, 2, 3`), and a combo lock of 3 digits: `[A][B][C]`

If we pick `A = 1`, `B` now has three values availables: `1`, `2`, and `3`, which is effectively `4 - A` values.
We them recurse: C has `(4 - B)` values, summed over all the possible values of B, which is `4 - A`, thus C overall takes `(4 - A) * (4 - B)`.

## 05

Could unfortunately not reuse `02` so I rewrote it. Jump handling is kind of crude but it does the job. It took me a stupid amount of time to understand inputs and outputs and I'm not sure I got them right.

## 06

To do. Had a hard time understanding the problem statement and that just pissed me off.

## 07

Done using intcode impl from 09 with minor adjustments (processing stalled until input is received)

## 08

Too lazy to even open AoC.

## 09

intcode is easy
