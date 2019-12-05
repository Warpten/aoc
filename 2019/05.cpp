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
    3,225,1,225,6,6,1100,1,238,225,104,0,1101,37,34,224,101,-71,224,224,4,224,1002,223,8,223,101,6,224,224,1,224,223,223,1002,113,50,224,1001,224,-2550,224,4,224,1002,223,8,223,101,2,224,224,1,223,224,223,1101,13,50,225,102,7,187,224,1001,224,-224,224,4,224,1002,223,8,223,1001,224,5,224,1,224,223,223,1101,79,72,225,1101,42,42,225,1102,46,76,224,101,-3496,224,224,4,224,102,8,223,223,101,5,224,224,1,223,224,223,1102,51,90,225,1101,11,91,225,1001,118,49,224,1001,224,-140,224,4,224,102,8,223,223,101,5,224,224,1,224,223,223,2,191,87,224,1001,224,-1218,224,4,224,1002,223,8,223,101,4,224,224,1,224,223,223,1,217,83,224,1001,224,-124,224,4,224,1002,223,8,223,101,5,224,224,1,223,224,223,1101,32,77,225,1101,29,80,225,101,93,58,224,1001,224,-143,224,4,224,102,8,223,223,1001,224,4,224,1,223,224,223,1101,45,69,225,4,223,99,0,0,0,677,0,0,0,0,0,0,0,0,0,0,0,1105,0,99999,1105,227,247,1105,1,99999,1005,227,99999,1005,0,256,1105,1,99999,1106,227,99999,1106,0,265,1105,1,99999,1006,0,99999,1006,227,274,1105,1,99999,1105,1,280,1105,1,99999,1,225,225,225,1101,294,0,0,105,1,0,1105,1,99999,1106,0,300,1105,1,99999,1,225,225,225,1101,314,0,0,106,0,0,1105,1,99999,7,226,226,224,102,2,223,223,1005,224,329,101,1,223,223,108,677,226,224,102,2,223,223,1005,224,344,1001,223,1,223,1108,226,677,224,102,2,223,223,1005,224,359,1001,223,1,223,8,677,226,224,102,2,223,223,1006,224,374,1001,223,1,223,107,226,226,224,102,2,223,223,1006,224,389,101,1,223,223,1108,677,226,224,1002,223,2,223,1005,224,404,1001,223,1,223,108,677,677,224,102,2,223,223,1005,224,419,101,1,223,223,7,226,677,224,1002,223,2,223,1006,224,434,1001,223,1,223,107,226,677,224,102,2,223,223,1005,224,449,101,1,223,223,1108,677,677,224,1002,223,2,223,1006,224,464,101,1,223,223,7,677,226,224,102,2,223,223,1006,224,479,101,1,223,223,1007,677,677,224,1002,223,2,223,1005,224,494,101,1,223,223,1008,226,226,224,102,2,223,223,1006,224,509,1001,223,1,223,107,677,677,224,102,2,223,223,1006,224,524,1001,223,1,223,8,226,226,224,1002,223,2,223,1005,224,539,1001,223,1,223,1007,677,226,224,102,2,223,223,1006,224,554,1001,223,1,223,1007,226,226,224,1002,223,2,223,1005,224,569,1001,223,1,223,8,226,677,224,1002,223,2,223,1006,224,584,101,1,223,223,108,226,226,224,1002,223,2,223,1006,224,599,101,1,223,223,1107,677,226,224,1002,223,2,223,1005,224,614,1001,223,1,223,1107,226,677,224,102,2,223,223,1006,224,629,1001,223,1,223,1008,226,677,224,102,2,223,223,1005,224,644,101,1,223,223,1107,226,226,224,102,2,223,223,1006,224,659,1001,223,1,223,1008,677,677,224,102,2,223,223,1006,224,674,1001,223,1,223,4,223,99,226
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
