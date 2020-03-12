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
using IntType = int64_t;

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
    // since max_param_count() will always be small, we just keep an array of that size
    // for each instruction, rather than use a vector and take a performance hit from
    // pointer indirections
    Parameter params[max_param_count()];
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

static std::vector<IntType> read_program_from_file(const char* filepath)
{
    std::vector<IntType> program;
    program.reserve(1024);

    std::ifstream infile(filepath);
    std::string istr;

    while (std::getline(infile, istr, ',')) {
        try {
            program.push_back(std::stoll(istr));
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
public:
    enum class State;

private:
    std::vector<IntType> m_memory;  // current memory state of IntCode machine
    size_t m_pc;                    // program counter
    State m_state;
    int m_relative_base;
    std::optional<int> m_input;

    inline void allocate_up_to(size_t address)
    {
        const size_t new_size_required = address + 1;
        if (m_memory.size() >= new_size_required) return;
        // extend program memory and fill new memory with zeros
        m_memory.resize(new_size_required, 0);
    }

    Instruction parse_next_instruction(void)
    {
        panic_if(m_pc >= m_memory.size(), "Program counter moved past end of memory.");

        Instruction inst;

        const int opcode = read_memory(m_pc);
        inst.code = opcode;
        inst.op = code_to_op(opcode % 100);
        panic_if(inst.op == Op::Unknown, "Unknown opcode encountered.");

        const int pcount = param_count(inst.op);
        assert(pcount >= 0 && pcount < 10);

        const int powers_of_ten[max_param_count()] = {100, 1000, 10000};

        for (int i = 0; i < pcount; i++) {
            const int mode_int = (opcode / powers_of_ten[i]) % 10;
            auto& mode = inst.params[i].mode;

            if (mode_int == 0) {
                mode = Parameter::Mode::Position;
            }
            else if (mode_int == 1) {
                mode = Parameter::Mode::Immediate;
            }
            else {
                assert(mode_int == 2);
                mode = Parameter::Mode::Relative;
            }

            inst.params[i].value = read_memory(m_pc + i + 1);
        }

        return inst;
    }

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
        : m_memory(program), m_pc(0), m_state(State::ReadyToBegin), m_relative_base(0), m_input({})
    {
    }

public:
    IntCodeVM(const char* filepath) : IntCodeVM(read_program_from_file(filepath))
    {
        allocate_up_to(2000);
    }

    enum class State { AwaitingInput, Halted, ReadyToBegin, Running };

    inline IntType read_memory(size_t address)
    {
        allocate_up_to(address);
        return m_memory[address];
    }

    inline void write_memory(size_t address, IntType value)
    {
        allocate_up_to(address);
        m_memory[address] = value;
    }

    State get_state(void) const { return m_state; }

    void set_input(IntType input) { m_input = input; }

    // return value: either empty on halt, or pauses the execution and returns a single
    // output
    std::optional<IntType> continue_execution(void)
    {
        assert(m_state != State::Halted);
        assert(!(!m_input && m_state == State::AwaitingInput));

        if (m_state == State::ReadyToBegin) m_state = State::Running;
        assert(m_state == State::Running || m_state == State::AwaitingInput);

        std::optional<IntType> output;
        while (true) {
            bool increment_pc_by_par_count = true;

            const Instruction inst = parse_next_instruction();
            if (m_state == State::AwaitingInput) assert(inst.op == Op::Input);

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
                if (!m_input) {
                    m_state = State::AwaitingInput;
                    return {};
                }

                const IntType out_addr = extract_output_parameter(inst.params[0]);
                write_memory(out_addr, *m_input);
                m_input = {};
                m_state = State::Running;
            }
            else if (inst.op == Op::Output) {
                // don't return output yet, we still need to increment the program counter
                // below.
                output = extract_parameter(inst.params[0]);
            }
            else if (inst.op == Op::Halt) {
                m_state = State::Halted;
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

