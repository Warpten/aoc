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
    // Copy intcode here
};


int main() {
    auto program = make_program(state);
    program.inputs.push_back(STEP);
    program.exec();
    for (auto&& output : program.outputs)
        std::cout << output << std::endl;
    return 0;
}
