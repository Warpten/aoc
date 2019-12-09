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
    mod_rel_base = 9,
    halt = 99,
};

enum operation_mode {
    position = 0,
    immediate = 1,
    relative = 2
};

using value_t = int64_t;

struct program_t;
struct opcode_t;

struct opcode_t {
    opcode_t(program_t& program, size_t eip);
    std::optional<opcode_t> exec(std::deque<value_t>& inputs, std::vector<value_t>& outputs);
    value_t& get_parameter(int32_t index);
    size_t get_parameter_count();

    void swap(opcode_t& other) {
        parameter_mode.swap(other.parameter_mode);
        code = other.code;
        eip = other.eip;
        std::swap(program, other.program);
    }

    opcode_t next();
    opcode_t jump_to(size_t abs_ofs);

    std::reference_wrapper<program_t> program;

    std::unordered_map<size_t, operation_mode> parameter_mode;
    intcode code;
    size_t eip;
};

struct program_t {
    program_t(value_t* program, size_t program_size);
    void exec();

    std::deque<value_t> inputs;
    std::vector<value_t> outputs;
    std::vector<value_t> ram;
    size_t rel_base = 0;

    opcode_t opc;
};

opcode_t::opcode_t(program_t& program, size_t eip)
    : eip(eip), program(program)
{
    value_t eip_instr = program.ram[eip];
    code = intcode(eip_instr - (eip_instr / 100) * 100);

    value_t parameter_modes = eip_instr / 100;
    for (int32_t i = 0; i < get_parameter_count(); ++i) {
        parameter_mode[i] = operation_mode(parameter_modes - (parameter_modes / 10) * 10);

        parameter_modes /= 10;
    }
}

size_t opcode_t::get_parameter_count() {
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
    case mod_rel_base:
        return 1;
    case halt:
        return 0;
    }

    throw std::runtime_error("not implemented");
}

value_t& opcode_t::get_parameter(int32_t index) {
    switch (parameter_mode[index]) {
        case position:
            return program.get().ram[program.get().ram[eip + 1 + index]];
        case immediate:
            return program.get().ram[eip + 1 + index];
            break;
        case relative:
            return program.get().ram[program.get().rel_base + program.get().ram[eip + 1 + index]];
            break;
        default:
            throw std::runtime_error("not implemented");
    }
}

std::optional<opcode_t> opcode_t::exec(std::deque<value_t>& inputs, std::vector<value_t>& outputs) {

    switch (code) {
    case add:
        //std::cout << "Executing opcode add " << eip << " { @" << get_parameter(2) << " = " << get_parameter(1) << " + " << get_parameter(0) << "}" << std::endl;
        get_parameter(2) = get_parameter(1) + get_parameter(0);
        break;
    case multiply:
        //std::cout << "Executing opcode multiply " << eip << " { @" << get_parameter(2) << " = " << get_parameter(1) << " + " << get_parameter(0) << "}" << std::endl;
        get_parameter(2) = get_parameter(1) * get_parameter(0);
        break;
    case load_input:
    {
        //std::cout << "Executing opcode load_input " << eip << " { @" << get_parameter(0) << " = " << inputs.front() << "}" << std::endl;
        auto input = inputs.front();
        inputs.pop_front();
        get_parameter(0) = input;
        break;
    }
    case write_output:
        //std::cout << "Executing opcode write_output " << eip << " { " << get_parameter(0) << "}" << std::endl;
        outputs.push_back(get_parameter(0));
        break;
    case less_than:
        //std::cout << "Executing opcode less_than " << eip << " { @" << get_parameter(2) << " = " << get_parameter(1) << " < " << get_parameter(0) << "}" << std::endl;
        get_parameter(2) = get_parameter(0) < get_parameter(1);
        break;
    case equals:
        //std::cout << "Executing opcode equals " << eip << " { @" << get_parameter(2) << " = " << get_parameter(1) << " == " << get_parameter(0) << "}" << std::endl;
        get_parameter(2) = get_parameter(0) == get_parameter(1);
        break;
    case jump_if_true:
        //std::cout << "Executing opcode jump_if_true " << eip << " { " << get_parameter(0) << "; @" << get_parameter(1) << "}" << std::endl;
        if (get_parameter(0) != 0)
            return jump_to(get_parameter(1)).exec(inputs, outputs);
        break;
    case jump_if_false:
        //std::cout << "Executing opcode jump_if_false " << eip << " { " << get_parameter(0) << "; @" << get_parameter(1) << "}" << std::endl;
        if (get_parameter(0) == 0)
            return jump_to(get_parameter(1)).exec(inputs, outputs);
        break;
    case mod_rel_base:
        //std::cout << "Executing opcode mod_rel_base " << eip << " { " << get_parameter(0) << "}" << std::endl;
        program.get().rel_base += get_parameter(0);
        break;
    case halt:
        return std::nullopt;
    }

    return next();
}

opcode_t opcode_t::next() {
    return opcode_t{ program, eip + 1 + get_parameter_count() };
}

opcode_t opcode_t::jump_to(size_t abs_ofs) {
    return opcode_t{ program, abs_ofs };
}

// program_t

program_t::program_t(value_t* program, size_t program_size)
    : opc(*this, 0), ram(program, program + program_size)
{
    ram.resize(std::max<size_t>(ram.size(), 0x8000u));
}

void program_t::exec() {
    std::optional<opcode_t> opt = opc.exec(inputs, outputs);
    while (opt) {
        auto next = (*opt).exec(inputs, outputs);
        if (!next)
            break;

        opt.swap(next);
    }
}

program_t make_program(value_t* program, size_t count) {
    return program_t{ program, count };
};

template <size_t N>
program_t make_program(value_t(&program)[N]) {
    return program_t{ program, N };
}

value_t state[] = {
    1102,34463338,34463338,63,1007,63,34463338,63,1005,63,53,1101,0,3,1000,109,988,209,12,9,1000,209,6,209,3,203,0,1008,1000,1,63,1005,63,65,1008,1000,2,63,1005,63,904,1008,1000,0,63,1005,63,58,4,25,104,0,99,4,0,104,0,99,4,17,104,0,99,0,0,1101,0,252,1023,1101,0,0,1020,1102,1,39,1013,1102,1,234,1029,1102,26,1,1016,1101,37,0,1005,1101,0,27,1011,1101,21,0,1000,1101,0,29,1019,1101,35,0,1003,1102,22,1,1007,1102,1,32,1001,1101,1,0,1021,1102,1,216,1027,1102,30,1,1012,1102,1,24,1009,1101,36,0,1002,1101,0,31,1010,1101,0,243,1028,1102,787,1,1024,1102,255,1,1022,1102,33,1,1017,1102,1,23,1004,1102,778,1,1025,1102,1,28,1008,1101,0,223,1026,1102,1,25,1015,1101,0,20,1006,1102,34,1,1014,1101,38,0,1018,109,-4,1202,5,1,63,1008,63,32,63,1005,63,203,4,187,1106,0,207,1001,64,1,64,1002,64,2,64,109,37,2106,0,-6,1001,64,1,64,1106,0,225,4,213,1002,64,2,64,109,3,2106,0,-8,4,231,1001,64,1,64,1105,1,243,1002,64,2,64,109,-12,2105,1,-1,1105,1,261,4,249,1001,64,1,64,1002,64,2,64,109,-13,2102,1,-3,63,1008,63,31,63,1005,63,285,1001,64,1,64,1106,0,287,4,267,1002,64,2,64,109,6,21102,40,1,0,1008,1017,40,63,1005,63,313,4,293,1001,64,1,64,1105,1,313,1002,64,2,64,109,-10,2107,31,-6,63,1005,63,331,4,319,1105,1,335,1001,64,1,64,1002,64,2,64,109,-6,2102,1,7,63,1008,63,28,63,1005,63,357,4,341,1105,1,361,1001,64,1,64,1002,64,2,64,109,2,21107,41,40,8,1005,1011,377,1106,0,383,4,367,1001,64,1,64,1002,64,2,64,109,-1,1201,2,0,63,1008,63,26,63,1005,63,403,1106,0,409,4,389,1001,64,1,64,1002,64,2,64,109,22,1205,-4,425,1001,64,1,64,1105,1,427,4,415,1002,64,2,64,109,-9,21101,42,0,3,1008,1018,39,63,1005,63,451,1001,64,1,64,1105,1,453,4,433,1002,64,2,64,109,3,21107,43,44,0,1005,1018,475,4,459,1001,64,1,64,1105,1,475,1002,64,2,64,109,-7,21101,44,0,0,1008,1011,44,63,1005,63,497,4,481,1105,1,501,1001,64,1,64,1002,64,2,64,109,17,1206,-7,513,1105,1,519,4,507,1001,64,1,64,1002,64,2,64,109,-24,1207,5,25,63,1005,63,537,4,525,1105,1,541,1001,64,1,64,1002,64,2,64,109,7,21108,45,43,2,1005,1013,557,1106,0,563,4,547,1001,64,1,64,1002,64,2,64,109,-5,1207,-3,34,63,1005,63,583,1001,64,1,64,1106,0,585,4,569,1002,64,2,64,109,5,21108,46,46,5,1005,1016,607,4,591,1001,64,1,64,1105,1,607,1002,64,2,64,109,-12,2108,20,8,63,1005,63,627,1001,64,1,64,1105,1,629,4,613,1002,64,2,64,109,24,1206,-3,647,4,635,1001,64,1,64,1105,1,647,1002,64,2,64,109,-30,2108,32,8,63,1005,63,665,4,653,1106,0,669,1001,64,1,64,1002,64,2,64,109,22,1208,-9,20,63,1005,63,691,4,675,1001,64,1,64,1106,0,691,1002,64,2,64,109,-4,21102,47,1,3,1008,1014,49,63,1005,63,715,1001,64,1,64,1105,1,717,4,697,1002,64,2,64,109,-10,2101,0,1,63,1008,63,36,63,1005,63,743,4,723,1001,64,1,64,1105,1,743,1002,64,2,64,109,16,1201,-9,0,63,1008,63,28,63,1005,63,769,4,749,1001,64,1,64,1105,1,769,1002,64,2,64,109,2,2105,1,5,4,775,1001,64,1,64,1106,0,787,1002,64,2,64,109,-5,1202,-6,1,63,1008,63,26,63,1005,63,807,1106,0,813,4,793,1001,64,1,64,1002,64,2,64,109,-16,2107,37,4,63,1005,63,833,1001,64,1,64,1105,1,835,4,819,1002,64,2,64,109,2,2101,0,1,63,1008,63,34,63,1005,63,855,1105,1,861,4,841,1001,64,1,64,1002,64,2,64,109,19,1205,2,875,4,867,1105,1,879,1001,64,1,64,1002,64,2,64,109,-2,1208,-8,23,63,1005,63,899,1001,64,1,64,1106,0,901,4,885,4,64,99,21101,0,27,1,21102,915,1,0,1106,0,922,21201,1,61455,1,204,1,99,109,3,1207,-2,3,63,1005,63,964,21201,-2,-1,1,21102,942,1,0,1105,1,922,22102,1,1,-1,21201,-2,-3,1,21102,1,957,0,1105,1,922,22201,1,-1,-2,1106,0,968,22101,0,-2,-2,109,-3,2105,1,0
};


int main() {
    auto program = make_program(state);
    program.inputs.push_back(STEP);
    program.exec();
    for (auto&& output : program.outputs)
        std::cout << output << std::endl;
    return 0;
}
