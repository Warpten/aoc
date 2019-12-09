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

    inline bool operator == (opcode_t const& o) { return o.eip == eip; }

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
    bool halted = false;
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
        if (inputs.empty())
            return *this;

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
        program.get().halted = true;
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
    opcode_t curr_instr = opc;

    std::optional<opcode_t> next_instr = curr_instr.exec(inputs, outputs);
    while (next_instr) {
        // Stalled for an input read, break out
        if (curr_instr == *next_instr)
            break;

        curr_instr = *next_instr;
        next_instr = curr_instr.exec(inputs, outputs);
    }

    // Not halted, set eip correctly
    if (!halted)
        opc = curr_instr;
}

program_t make_program(value_t* program, size_t count) {
    return program_t{ program, count };
};

template <size_t N>
program_t make_program(value_t(&program)[N]) {
    return program_t{ program, N };
}

value_t state[] = {
    3,8,1001,8,10,8,105,1,0,0,21,38,47,64,89,110,191,272,353,434,99999,3,9,101,4,9,9,102,3,9,9,101,5,9,9,4,9,99,3,9,1002,9,5,9,4,9,99,3,9,101,2,9,9,102,5,9,9,1001,9,5,9,4,9,99,3,9,1001,9,5,9,102,4,9,9,1001,9,5,9,1002,9,2,9,1001,9,3,9,4,9,99,3,9,102,2,9,9,101,4,9,9,1002,9,4,9,1001,9,4,9,4,9,99,3,9,101,1,9,9,4,9,3,9,101,1,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,101,2,9,9,4,9,3,9,101,1,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,101,2,9,9,4,9,99,3,9,101,2,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,101,2,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,2,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,2,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,102,2,9,9,4,9,99,3,9,1001,9,2,9,4,9,3,9,1001,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1001,9,1,9,4,9,3,9,1002,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,1002,9,2,9,4,9,3,9,101,1,9,9,4,9,3,9,101,1,9,9,4,9,99,3,9,102,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1001,9,1,9,4,9,3,9,1002,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1001,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,1,9,4,9,3,9,1002,9,2,9,4,9,99,3,9,101,1,9,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,1001,9,2,9,4,9,3,9,102,2,9,9,4,9,3,9,102,2,9,9,4,9,3,9,1001,9,2,9,4,9,3,9,1002,9,2,9,4,9,3,9,1002,9,2,9,4,9,3,9,1002,9,2,9,4,9,99
    //3,26,1001,26,-4,26,3,27,1002,27,2,27,1,27,26,27,4,27,1001,28,-1,28,1005,28,6,99,0,0,5
};

struct amp_loop {
    program_t a;
    program_t b;
    program_t c;
    program_t d;
    program_t e;

    value_t thrust;

    amp_loop(value_t pa, value_t pb, value_t pc, value_t pd, value_t pe)
        : a(make_program(state)),
        b(make_program(state)),
        c(make_program(state)),
        d(make_program(state)),
        e(make_program(state))
    {
        value_t feedback = 0;

        a.inputs.push_back(pa);
        b.inputs.push_back(pb);
        c.inputs.push_back(pc);
        d.inputs.push_back(pd);
        e.inputs.push_back(pe);

        size_t cnt = 2;

        while (!e.halted) {
            a.inputs.push_back(feedback);
            a.exec();

            b.inputs.push_back(a.outputs.back());
            b.exec();

            c.inputs.push_back(b.outputs.back());
            c.exec();

            d.inputs.push_back(c.outputs.back());
            d.exec();

            e.inputs.push_back(d.outputs.back());
            e.exec();

            feedback = e.outputs.back();
        }

        thrust = feedback;
    }
};


int main() {
    size_t phases[] = { 5, 6, 7, 8, 9 };

    value_t thrust = 0;

    for (auto pa : phases) {
        for (auto pb : phases) {
            if (pb == pa)
                continue;

            for (auto pc : phases) {
                if (pc == pa || pc == pb)
                    continue;

                for (auto pd : phases) {
                    if (pd == pa || pd == pb || pd == pc)
                        continue;

                    for (auto pe : phases) {
                        if (pe == pa || pe == pb || pe == pc || pe == pd)
                            continue;

                        amp_loop loop(pa, pb, pc, pd, pe);
                        thrust = std::max(thrust, loop.thrust);
                    }
                }
            }
        }
    }

    std::cout << thrust;
    return 0;
}
