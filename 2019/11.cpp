
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

namespace computer {
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
        program_t(const value_t* program, size_t program_size);
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

    program_t::program_t(const value_t* program, size_t program_size)
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
}


struct coordinate {
    int32_t x, y;

    coordinate(int32_t x, int32_t y) : x(x), y(y) { }
};

bool operator == (coordinate const& lhs, coordinate const& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

namespace std
{
    template <> struct hash<coordinate> {
        size_t operator()(coordinate const& c) const {
            return std::hash<int32_t>()(c.x) ^ std::hash<int32_t>()(c.y);
        }
    };
}

struct hull_t {
    struct panel_t {
        enum color_t { black = 1, white = 0 };

        color_t color;
    };

    std::unordered_map<coordinate, panel_t> panels;

    panel_t::color_t get_color(int32_t x, int32_t y) {
        return panels[coordinate{ x, y }].color;
    }

    void set_color(int32_t x, int32_t y, panel_t::color_t color) {
        panels[{x, y}].color = color;
    }
};

struct roombat_t {
    enum direction_t {
        turn_left = 0,
        turn_right = 1
    };

    enum facing_t {
        north,
        south,
        east,
        west
    };

    computer::program_t brain;
    coordinate position;
    facing_t facing;
    hull_t hull;

    template <size_t N>
    roombat_t(computer::value_t const (&p)[N]) : brain(p, N), position(0, 0), facing(north) { }

    hull_t::panel_t::color_t get_current_color() {
        return hull.get_color(position.x, position.y);
    }

    void move(int32_t xd, int32_t yd) {
        position.x += xd;
        position.y += yd;
    }

    bool step_once() {
        brain.inputs.clear();
        brain.inputs.push_back(get_current_color() == hull_t::panel_t::color_t::black ? 0 : 1);
        brain.outputs.clear();
        brain.exec();

        auto new_color = hull_t::panel_t::color_t(brain.outputs.front());
        auto direction = direction_t(brain.outputs.back());

        hull.set_color(position.x, position.y, new_color);
        switch (facing) {
            case north:
                position.x += direction == direction_t::turn_left ? -1 : +1;
                facing = direction == direction_t::turn_left ? facing_t::west : facing_t::east;
                break;
            case south:
                position.x += direction == direction_t::turn_left ? +1 : -1;
                facing = direction == direction_t::turn_left ? facing_t::east : facing_t::west;
                break;
            case east:
                position.y += direction == direction_t::turn_left ? -1 : +1;
                facing = direction == direction_t::turn_left ? facing_t::north : facing_t::south;
                break;
            case west:
                position.y += direction == direction_t::turn_left ? +1 : -1;
                facing = direction == direction_t::turn_left ? facing_t::south : facing_t::north;
                break;
            default:
                throw std::runtime_error("out of range");
        }

        return brain.halted;
    }

    size_t painted_panel_count() const {
        return hull.panels.size();
    }
};

computer::value_t state[] = {
    // again, copy intcode here
};

#define STEP 2

int main() {
#if STEP == 1
    roombat_t roombat(state);
    while (!roombat.step_once());
    std::cout << roombat.painted_panel_count();
#elif STEP == 2
    roombat_t roombat(state);
    roombat.hull.set_color(roombat.position.x, roombat.position.y, hull_t::panel_t::color_t::white);
    while (!roombat.step_once());

    auto hull = roombat.hull;
    size_t min_x = std::numeric_limits<size_t>::max();
    size_t max_x = std::numeric_limits<size_t>::min();

    size_t min_y = std::numeric_limits<size_t>::max();
    size_t max_y = std::numeric_limits<size_t>::min();

    for (auto const& kv : hull.panels)
    {
        min_x = std::min<size_t>(kv.first.x, min_x);
        max_x = std::max<size_t>(kv.first.x, max_x);

        min_y = std::min<size_t>(kv.first.y, min_y);
        max_y = std::max<size_t>(kv.first.y, max_y);
    }

    for (size_t y = min_y; y <= max_y; ++y) {
        for (size_t x = min_x; x <= max_x; ++x) {
            auto hull_color = roombat.hull.get_color(x, y);
            if (hull_color == hull_t::panel_t::color_t::black)
                std::cout << "*";
            else
                std::cout << " ";
        }
        std::cout << std::endl;
    }
#endif

    return 0;
}
