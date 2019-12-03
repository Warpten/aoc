# aoc

All samples designed to run on godbolt/clang-trunk

## 01

Nothing to say.

## 02

Using `unordered_map` is useless but who knows, might save me if this rocket state machine is needed again.

## 03

Took me a short while because i tried `std::vector` first which does not have constant or logarithmic search time.
Also debated storing segments instead of points but problem input is small enough that that's irrelevant.
