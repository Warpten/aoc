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

template <typename T>
struct vec2 {
    T x;
    T y;

    vec2(T v) : vec2(v, v) { }

    vec2(T x, T y) {
        this->x = x;
        this->y = y;
    }

    vec2 scale(T s) {
        return { x * s, y * s };
    }

    vec2 norm() const {
        if (lengthSquared() == 0)
            return *this;

        return { x / length(), y / length() };
    }

    vec2 round(size_t precision) const {
        auto p = std::pow(10, precision);
        return { T(int64_t(x * p) / p), T(int64_t(y * p) / p) };
    }

    T length() const {
        return std::sqrt(x * x + y * y);
    }

    T lengthSquared() const {
        return x * x + y * y;
    }

    bool operator == (vec2 const& o) const {
        return x == o.x && y == o.y;
    }
};

template <typename T>
vec2<T> operator - (vec2<T> const& l, vec2<T> const& r) {
    vec2<T> c(l);
    c.x -= r.x;
    c.y -= r.y;
    return c;
}

template <typename T>
vec2<T> operator + (vec2<T> const& l, vec2<T> const& r) {
    vec2<T> c(l);
    c.x += r.x;
    c.y += r.y;
    return c;
}

template <typename T>
vec2<T> operator * (vec2<T> const& l, vec2<T> const& r) {
    vec2<T> c(l);
    c.x *= r.x;
    c.y *= r.y;
    return c;
}

using vec2d = vec2<double>;
using vec2i = vec2<int32_t>;
using vec2f = vec2<float>;

namespace std {
    template <typename T>
    struct hash<vec2<T>> {
        size_t operator()(vec2<T> const& c) const {
            return std::hash<T>()(std::round(c.x) * 100000.0f)
                ^ std::hash<T>()(std::round(c.y) * 100000.0f);
        }
    };
}

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
    program_t(value_t const* program, size_t program_size);

    
private:// Horrible hack to properly initialize stuff
    program_t& load_ram(value_t const* program, size_t program_size);
public:

    void exec();
    void reset();

    std::deque<value_t> inputs;
    std::vector<value_t> outputs;
    std::unordered_map<size_t, value_t> ram;
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

program_t::program_t(value_t const* program, size_t program_size) : opc(load_ram(program, program_size), 0)
{
    reset();
}

program_t& program_t::load_ram(value_t const* program, size_t program_size) {
    for (size_t i = 0; i < program_size; ++i)
        ram[i] = program[i];

    return *this;
}

void program_t::reset() {
    opc = opcode_t(*this, 0);
    inputs.clear();
    outputs.clear();
    halted = false;
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
   /* copy your intcode here */
};

enum block_type_t {
    air = 0,
    wall = 1,
    block = 2,
    paddle = 3,
    ball = 4,
};

// #define ENABLE_DUMP_BOARD

struct arcade_t {
    program_t program;

    int32_t score = 0;
    int32_t block_count = 0;

    vec2i paddle;
    vec2i ball;

#ifdef ENABLE_DUMP_BOARD
    int32_t minx;
    int32_t maxx;
    int32_t miny;
    int32_t maxy;
    std::unordered_map<vec2i, block_type_t> board;
#endif

    template <size_t N>
    arcade_t(value_t const (&value)[N]) : program(value, N), paddle(0), ball(0) {
        program.exec();

#ifdef ENABLE_DUMP_BOARD
        minx = std::numeric_limits<int32_t>::max();
        miny = std::numeric_limits<int32_t>::max();
        maxx = std::numeric_limits<int32_t>::min();
        maxy = std::numeric_limits<int32_t>::min();
#endif
        for (size_t i = 0; i < program.outputs.size(); i += 3) {
            int32_t x = program.outputs[i + 0];
            int32_t y = program.outputs[i + 1];
            block_type_t tile = block_type_t(program.outputs[i + 2]);

            if (x == -1 && y == 0) {
                score = program.outputs[i + 2];
            }
            else {
#ifdef ENABLE_DUMP_BOARD
                minx = std::min(minx, x);
                maxx = std::max(maxx, x);

                miny = std::min(miny, y);
                maxy = std::max(maxy, y);
#endif

                if (tile == block_type_t::block)
                    ++block_count;
                else if (tile == block_type_t::ball)
                    ball = { x, y };
                else if (tile == block_type_t::paddle)
                    paddle = { x, y };

#ifdef ENABLE_DUMP_BOARD
                board[{x, y}] = tile;
#endif
            }
        }
    }

    void dump() {
#ifdef ENABLE_DUMP_BOARD
        std::cout.flush();
        system("cls");

        for (int32_t y = miny; y <= maxy; ++y) {
            for (int32_t x = minx; x <= maxx; ++x) {
                switch (board[{x, y}]) {
                case block_type_t::air:
                    std::cout << ' ';
                    break;
                case block_type_t::wall:
                    std::cout << '#';
                    break;
                case block_type_t::block:
                    std::cout << '$';
                    break;
                case block_type_t::paddle:
                    std::cout << '=';
                    break;
                case block_type_t::ball:
                    std::cout << '+';
                    break;
                }
            }
            std::cout << std::endl;
        }
#endif
    }

    void reset() {
        program.reset();
    }

    void step() {
        auto signof = [](int32_t v) {
            if (v > 0) return +1;
            if (v < 0) return -1;
            return 0;
        };

        int32_t input = signof(ball.x - paddle.x);
        program.reset();
        program.inputs.push_back(input);
        program.exec();

        size_t prev_blocks = block_count;
        block_count = 0;

        for (size_t i = 0; i < program.outputs.size(); i += 3) {
            int32_t x = program.outputs[i + 0];
            int32_t y = program.outputs[i + 1];

            if (x == -1 && y == 0) {
                score = program.outputs[i + 2];
            }
            else {
                block_type_t tile = block_type_t(program.outputs[i + 2]);
#ifdef ENABLE_DUMP_BOARD
                minx = std::min(minx, x);
                maxx = std::max(maxx, x);

                miny = std::min(miny, y);
                maxy = std::max(maxy, y);

                board[{x, y}] = tile;
#endif

                if (tile == block_type_t::paddle)
                    paddle = { x, y };
                else if (tile == block_type_t::block)
                    ++block_count;
                else if (tile == block_type_t::ball)
                    ball = { x, y };
            }

        }

#ifdef ENABLE_DUMP_BOARD
        dump();
#endif

        // std::cout << "Ball at " << ball.x << ", paddle at " << paddle.x << ", moving " << (input == -1 ? "left" : (input == 1 ? "right" : "not")) << std::endl;
        if (block_count != prev_blocks) {
            std::cout << "Score: " << score << ", " << block_count << " blocks remaining.\r\n";
            prev_blocks = block_count;
        }
        program.outputs.clear();
    }
};

int main() {
    arcade_t arcade(state);
    arcade.dump();
    std::cout << "Blocks to break: " << arcade.block_count << std::endl;

    arcade.reset();
    arcade.program.ram[0] = 2; // Free play, wheeeee!

    while (arcade.block_count > 0 && !arcade.program.halted) {
        arcade.step();
        // arcade.dump();
    }

    std::cout << "Final score: " << arcade.score;

    return 0;
}
