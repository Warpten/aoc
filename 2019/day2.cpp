#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <stdexcept>

using size_t = std::size_t;

#define STEP 2

enum intcode
{
    add = 1,
    multiply = 2,
    halt = 99
};

using memory_t = std::unordered_map<size_t, size_t>;

struct rocket {
    size_t handle_opcode(intcode code, size_t l, size_t r, size_t o) {
        switch (code) {
            case add:
                ram[o] = ram[l] + ram[r];
                return 4;
            case multiply:
                ram[o] = ram[l] * ram[r];
                return 4;
            case halt:
                return 1;
        }

        throw std::runtime_error("unhandled opc");
    }

    template <size_t N>
    rocket(const size_t (&state)[N]) {
        for (size_t i = 0; i < N; ++i)
            ram[i] = state[i];
    }

    void patch(size_t ofs, size_t v) {
        ram[ofs] = v;
    }

    void run() {
        size_t position = 0;
        while (true) {
            intcode op = (intcode) ram[position];
            if (op == halt)
                break;

            size_t l = ram[position + 1];
            size_t r = ram[position + 2];
            size_t o = ram[position + 3];

            position += handle_opcode(op, l, r, o);
        }
    }

    size_t read_memory(size_t position) { return ram[position]; }

private:
    memory_t ram;
};

const size_t state[] = {
    1,0,0,3,1,1,2,3,1,3,4,3,1,5,0,3,2,6,1,19,1,19,5,23,2,10,23,27,2,27,13,31,
    1,10,31,35,1,35,9,39,2,39,13,43,1,43,5,47,1,47,6,51,2,6,51,55,1,5,55,59,
    2,9,59,63,2,6,63,67,1,13,67,71,1,9,71,75,2,13,75,79,1,79,10,83,2,83,9,87,
    1,5,87,91,2,91,6,95,2,13,95,99,1,99,5,103,1,103,2,107,1,107,10,0,99,2,0,
    14,0
};


int main() {
#if STEP == 1
    rocket rocket(state);
    rocket.patch(1, 12);
    rocket.patch(2, 2);
    rocket.run();
    std::cout << rocket.read_memory(0);
#elif STEP == 2
    constexpr const static size_t target = 19690720;
    for (size_t noun = 1; noun <= 99; ++noun) {
        for (size_t verb = 1; verb <= 99; ++verb) {
            rocket rocket(state);
            rocket.patch(1, noun);
            rocket.patch(2, verb);
            rocket.run();
            if (rocket.read_memory(0) == target) {
                std::cout << 100 * noun + verb << std::endl;
                return 0;
            }
        }
    }
#endif
    return 0;
}
