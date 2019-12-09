#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "magic_enum.hpp"

namespace intcode_detail {

// as of day 9, we need to support very large numbers
using IntType = __int128;

enum class Op {
    Addition,
    Multiplication,
    Input,
    Output,
    Halt,
    JumpIfTrue,
    JumpIfFalse,
    LessThan,
    Equals,
    ModifyRelativeBase,
    Unknown
};

static constexpr Op code_to_op(int code)
{
    switch (code) {
        case 1:
            return Op::Addition;
        case 2:
            return Op::Multiplication;
        case 3:
            return Op::Input;
        case 4:
            return Op::Output;
        case 5:
            return Op::JumpIfTrue;
        case 6:
            return Op::JumpIfFalse;
        case 7:
            return Op::LessThan;
        case 8:
            return Op::Equals;
        case 9:
            return Op::ModifyRelativeBase;
        case 99:
            return Op::Halt;
        default:
            return Op::Unknown;
    };
}

static constexpr int param_count(Op op)
{
    switch (op) {
        case Op::Addition:
            return 3;
        case Op::Multiplication:
            return 3;
        case Op::Input:
            return 1;
        case Op::Output:
            return 1;
        case Op::JumpIfTrue:
            return 2;
        case Op::JumpIfFalse:
            return 2;
        case Op::LessThan:
            return 3;
        case Op::Equals:
            return 3;
        case Op::Halt:
            return 0;
        case Op::ModifyRelativeBase:
            return 1;
        case Op::Unknown:
            return -1;
    };
}

constexpr size_t max_param_count(void)
{
    int maximum = 0;
    for (auto op : magic_enum::enum_values<Op>()) {
        maximum = std::max(param_count(op), maximum);
    };
    return maximum;
}

struct Parameter {
    enum class Mode { Position, Immediate, Relative } mode;
    IntType value;
};

struct Instruction {
    // since max_param_count() will always be small, we just keep an array of that size for each
    // instruction, rather than use a vector and take a performance hit from pointer indirections
    std::array<Parameter, max_param_count()> params;
    Op op;
    IntType code;
};

static void panic_if(bool condition, const char* msg)
{
    if (condition) {
        std::cerr << "INTCODE FATAL ERROR: " << msg << std::endl;
        exit(EXIT_FAILURE);
    }
};

// from https://stackoverflow.com/questions/45608424/atoi-for-int128-t-type
__int128 stoi128(const std::string& str)
{
    const char* s = str.c_str();

    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '+') ++s;
    int sign = 1;
    if (*s == '-') {
        ++s;
        sign = -1;
    }
    size_t digits = 0;
    while (s[digits] >= '0' && s[digits] <= '9') ++digits;
    char scratch[digits];
    for (size_t i = 0; i < digits; ++i) scratch[i] = s[i] - '0';
    size_t scanstart = 0;

    __int128 result = 0;
    __int128 mask = 1;
    while (scanstart < digits) {
        if (scratch[digits - 1] & 1) result |= mask;
        mask <<= 1;
        for (size_t i = digits - 1; i > scanstart; --i) {
            scratch[i] >>= 1;
            if (scratch[i - 1] & 1) scratch[i] |= 8;
        }
        scratch[scanstart] >>= 1;
        while (scanstart < digits && !scratch[scanstart]) ++scanstart;
        for (size_t i = scanstart; i < digits; ++i) {
            if (scratch[i] > 7) scratch[i] -= 3;
        }
    }

    return result * sign;
}

// inefficient, but we rarely call it
std::string int128_to_string(__int128 num)
{
    std::string str;
    do {
        int digit = num % 10;
        str = std::to_string(digit) + str;
        num = (num - digit) / 10;
    } while (num != 0);
    return str;
}

static std::vector<IntType> read_program_from_file(const char* filepath)
{
    std::vector<IntType> program;
    program.reserve(256);

    std::ifstream infile(filepath);
    std::string istr;

    while (std::getline(infile, istr, ',')) {
        try {
            program.push_back(stoi128(istr));
        }
        catch (...) {
            std::cerr << "Failed to parse program integer input: " << istr << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return program;
}

}  // namespace intcode_detail

using namespace intcode_detail;

class IntCodeVM {
    std::vector<IntType> m_state;  // current memory state of IntCode machine
    size_t m_pc;                   // program counter
    bool m_halted;                 // 'true' if HALT op code encountered during past execution
    int m_relative_base;

    inline void allocate_up_to(size_t address)
    {
        const size_t new_size_required = address + 1;
        if (m_state.size() >= new_size_required) return;
        // extend program memory and fill new memory with zeros
        m_state.resize(new_size_required, 0);
    }

    inline IntType read_memory(size_t address)
    {
        allocate_up_to(address);
        return m_state[address];
    }

    inline void write_memory(size_t address, IntType value)
    {
        allocate_up_to(address);
        m_state[address] = value;
    }

    Instruction parse_next_instruction(void)
    {
        panic_if(m_pc >= m_state.size(), "Program counter moved past end of memory.");

        Instruction inst;

        const int opcode = read_memory(m_pc);
        inst.code = opcode;
        inst.op = code_to_op(opcode % 100);
        panic_if(inst.op == Op::Unknown, "Unknown opcode encountered.");

        auto int_to_parameter_mode = [](int i) {
            if (i == 0) {
                return Parameter::Mode::Position;
            }
            else if (i == 1) {
                return Parameter::Mode::Immediate;
            }
            else {
                panic_if(i != 2, "Invalid parameter mode specifier encountered.");
                return Parameter::Mode::Relative;
            }
        };

        const int pcount = param_count(inst.op);
        assert(pcount >= 0);

        for (int i = 0; i < pcount; i++) {
            const int accum = static_cast<int>(std::pow(10, i + 2));
            inst.params[i].mode = int_to_parameter_mode((opcode / accum) % 10);
            inst.params[i].value = read_memory(m_pc + i + 1);
        }

        return inst;
    }

    // TODO: move this to Instruction struct member and do run-time checking of parameter index
    inline IntType extract_parameter(Parameter param)
    {
        switch (param.mode) {
            case Parameter::Mode::Immediate:
                return param.value;
            case Parameter::Mode::Position:
                return read_memory(param.value);
            case Parameter::Mode::Relative:
                return read_memory(m_relative_base + param.value);
        }
    };

    // output address parameters have different mode rules than normal op parameters
    inline IntType extract_output_parameter(Parameter param)
    {
        assert(param.mode != Parameter::Mode::Immediate);

        if (param.mode == Parameter::Mode::Position) {
            return param.value;
        }
        else {
            return m_relative_base + param.value;
        }
    };

    IntCodeVM(const std::vector<IntType>& program)
        : m_state(program), m_pc(0), m_halted(false), m_relative_base(0)
    {
    }

public:
    IntCodeVM(const char* filepath) : IntCodeVM(read_program_from_file(filepath)) {}

    void run_until_halt_with_single_input(IntType user_input)
    {
        auto start_time = std::chrono::steady_clock::now();

        std::optional<IntType> output = continue_execution(user_input);

        while (output) {
            std::cout << int128_to_string(*output) << std::endl;
            output = continue_execution({});
        }

        auto end_time = std::chrono::steady_clock::now();

        std::cout << "computation time: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
                  << "us" << std::endl;
    }

    // return value: either empty on halt, or pauses the execution and returns a single output
    std::optional<IntType> continue_execution(std::optional<IntType> input)
    {
        // once the program has halted, it should never be asked to resume execution again
        panic_if(m_halted, "Attempted to resume execution of halted program.");

        // Input should only be asked for once during each execution, so we keep track to check this
        bool input_instruction_encountered = false;

        std::optional<IntType> output;

        while (true) {
            bool increment_pc_by_par_count = true;

            const Instruction inst = parse_next_instruction();

            // TODO: switch on parameter count to reduce code duplication
            if (inst.op == Op::Addition || inst.op == Op::Multiplication) {
                const IntType x = extract_parameter(inst.params[0]);
                const IntType y = extract_parameter(inst.params[1]);
                const IntType out_addr = extract_output_parameter(inst.params[2]);

                if (inst.op == Op::Addition) {
                    write_memory(out_addr, x + y);
                }
                else {
                    assert(inst.op == Op::Multiplication);
                    write_memory(out_addr, x * y);
                }
            }
            else if (inst.op == Op::Input) {
                assert(!input_instruction_encountered);
                input_instruction_encountered = true;

                const IntType out_addr = extract_output_parameter(inst.params[0]);

                // input should never be empty when input instruction encountered
                assert(input);
                write_memory(out_addr, *input);
            }
            else if (inst.op == Op::Output) {
                // don't return output yet, we still need to increment the program counter below.
                output = extract_parameter(inst.params[0]);
            }
            else if (inst.op == Op::Halt) {
                m_halted = true;
                return {};
            }
            else if (inst.op == Op::JumpIfTrue) {
                const IntType x = extract_parameter(inst.params[0]);
                const IntType y = extract_parameter(inst.params[1]);

                if (x != 0) {
                    m_pc = y;
                    increment_pc_by_par_count = false;
                }
            }
            else if (inst.op == Op::JumpIfFalse) {
                const IntType x = extract_parameter(inst.params[0]);
                const IntType y = extract_parameter(inst.params[1]);

                if (x == 0) {
                    m_pc = y;
                    increment_pc_by_par_count = false;
                }
            }
            else if (inst.op == Op::LessThan) {
                const IntType x = extract_parameter(inst.params[0]);
                const IntType y = extract_parameter(inst.params[1]);
                const IntType out_addr = extract_output_parameter(inst.params[2]);

                write_memory(out_addr, x < y ? 1 : 0);
            }
            else if (inst.op == Op::Equals) {
                const IntType x = extract_parameter(inst.params[0]);
                const IntType y = extract_parameter(inst.params[1]);
                const IntType out_addr = extract_output_parameter(inst.params[2]);

                write_memory(out_addr, x == y ? 1 : 0);
            }
            else if (inst.op == Op::ModifyRelativeBase) {
                const IntType x = extract_parameter(inst.params[0]);
                m_relative_base += x;
            }
            else {
                assert(false && "Invalid opcode encountered");
            }

            if (increment_pc_by_par_count) {
                m_pc += param_count(inst.op) + 1;
            }

            if (output) return output;
        }
    }
};

