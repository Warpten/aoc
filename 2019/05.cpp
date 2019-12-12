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
    load_input = 3,
    write_output = 4,
    jump_if_true = 5,
    jump_if_false = 6,
    less_than = 7,
    equals = 8,
    halt = 99,
};

enum operation_mode {
    position = 0,
    immediate = 1
};

struct opcode_t {
    opcode_t(int32_t input, int32_t output, int32_t* eip, int32_t* ram) : eip(eip), ram(ram), input(input), output(output) {
        code = intcode(*eip - (*eip / 100) * 100);

        int32_t parameter_modes = *eip / 100;
        for (int32_t i = 0; i < get_parameter_count(); ++i) {
            parameter_mode[i] = operation_mode(parameter_modes - (parameter_modes / 10) * 10);

            parameter_modes /= 10;
        }
    }

    std::unordered_map<size_t, operation_mode> parameter_mode;
    intcode code;
    int32_t output;
    int32_t input;
    int32_t* eip;
    int32_t* ram;


    int32_t& get_parameter(int32_t index) {
        switch (parameter_mode[index]) {
        case position:
            return ram[eip[index + 1]];
        case immediate:
            return eip[index + 1];
        }

        throw std::runtime_error("unknown mode");
    }

    size_t get_parameter_count() {
        switch (code) {
            case add:
            case multiply:
            case less_than:
            case equals:
                return 3;
            case load_input:
            case write_output:
                return 1;
            case jump_if_false:
            case jump_if_true:
                return 2;
            case halt:
                return 0;
        }
    }

    int32_t exec() {
        switch (code) {
        case add:
            if (parameter_mode[2] != position)
                throw std::runtime_error("target must be position mode");
            get_parameter(2) = get_parameter(1) + get_parameter(0);
            break;
        case multiply:
            if (parameter_mode[2] != position)
                throw std::runtime_error("target must be position mode");
            get_parameter(2) = get_parameter(1) * get_parameter(0);
            break;
        case load_input:
            if (parameter_mode[0] != position)
                throw std::runtime_error("target must be position mode");
            get_parameter(0) = input;
            break;
        case write_output:
            output = get_parameter(0);
            break;
        case less_than:
            get_parameter(2) = get_parameter(0) < get_parameter(1);
            break;
        case equals:
            get_parameter(2) = get_parameter(0) == get_parameter(1);
            break;
        case jump_if_true:
            if (get_parameter(0) != 0)
                return opcode_t{ input, output, ram + get_parameter(1), ram }.exec();
            break;
        case jump_if_false:
            if (get_parameter(0) == 0)
                return opcode_t{ input, output, ram + get_parameter(1), ram }.exec();
            break;
        case halt:
            return output;
        }

        return next().exec();
    }

    opcode_t next() {
        return opcode_t{ input, output, eip + get_parameter_count() + 1, ram };
    }
};

opcode_t make_program(int32_t* program, int32_t input) {
    return opcode_t{ input, 0, program, program };
};

int32_t state[] = {
    // copy intcode here
};


int main() {
    auto program = make_program(state, 
#if STEP == 1
        1
#elif STEP == 2
        5
#endif
    );
    std::cout << program.exec();
    return 0;
}
