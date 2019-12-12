#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <tuple>

#include <iostream>

using size_t = std::size_t;

#define STEP 2

struct keypad {
    struct digit {
        int32_t value;
        size_t idx;

        digit& next(keypad& kp) {
            return kp[idx + 1];
        }

        digit& prev(keypad& kp) {
            return kp[idx - 1];
        }
    };

    keypad(int32_t init) {
        for (size_t p10 = 0; p10 < 6; ++p10) {
            int32_t pmin = init / std::pow(10, 5 - p10);

            digits[5 - p10].value = pmin;
            digits[5 - p10].idx = p10;

            init -= pmin * std::pow(10, 5 - p10);
        }
    }

    inline operator int32_t() const {
        int32_t acc = 0;
        int32_t itr = 0;
        for (auto&& digit : digits)
            acc += digit.value * std::pow(10, itr++);
        return acc;
    }

    inline       digit& operator[] (size_t idx)       { return digits[digits.size() - 1 - idx]; }
    inline const digit& operator[] (size_t idx) const { return digits[digits.size() - 1 - idx]; }

    inline operator bool() const {
        // There has to be at least one pair
        bool pair_found = false;
        for (size_t i = 0; i < digits.size() - 1;) {
#if STEP == 1
            pair_found = (*this)[i].value == (*this)[i + 1].value;
            if (pair_found)
                break;
            ++i;
#elif STEP == 2
            size_t group_size = 1;
            while (i + group_size < digits.size()) {
                if ((*this)[i].value != (*this)[i + group_size].value)
                    break;

                ++group_size;
            }

            pair_found = pair_found || group_size == 2;
            i += group_size - 1;
#endif
        }

        if (!pair_found)
            return false;

        // Digits never decrease, but they can keep the same value
        char last_checked_value = (*this)[0].value;
        size_t itr = 1;
        while (itr < digits.size()) {
            if ((*this)[itr].value < last_checked_value)
                return false;

            last_checked_value = (*this)[itr].value;
            ++itr;
        }

        return true;
    }

    void increment(size_t idx = 0) {
        digit& current = digits[idx];
        bool carry = (current.value + 1 > 9);
        current.value = carry ? 0 : current.value + 1;

        if (idx + 1 < digits.size() && carry)
            increment(idx + 1);
    }

    // Stored LTR in memory
    std::array<digit, 6> digits;
};


int main() {

    // Lower bound is given as 347312 but as per problem
    // statement that is an invalid combo lock.
    // Since 7 > 3, has to be at least 347777.
    keypad keypad(/* copy lower boundary here */);

    // Upper bound is given as 805915 but as per problem
    // statement that is an invalid combo lock.
    // since 8 > 0, has to be 799999

    size_t valid_count = 0;
    while (static_cast<int32_t>(keypad) <= /* copy higher boundary her*/)
    {
        if (keypad) {
            std::cout << static_cast<int>(keypad) << " ";
            ++valid_count;
        }
        keypad.increment();
    }

    std::cout << valid_count;
    return 0;
}
