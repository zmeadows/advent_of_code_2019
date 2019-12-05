#include <assert.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

enum class ParameterMode { Position, Immediate };

// remember to update ALL_OPS if you add new op codes!!!
enum class Op {
    Addition,
    Multiplication,
    Input,
    Output,
    Halt,
    JumpIfTrue,
    JumpIfFalse,
    LessThan,
    Equals
};

constexpr Op ALL_OPS[9] = {Op::Addition,   Op::Multiplication, Op::Input,    Op::Output, Op::Halt,
                           Op::JumpIfTrue, Op::JumpIfFalse,    Op::LessThan, Op::Equals};

std::optional<Op> CODE_TO_OP(int code)
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
        case 99:
            return Op::Halt;
        default:
            return {};
    };
}

constexpr int PARAM_COUNT(Op op)
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
    };
}

constexpr int MAX_PARAM_COUNT(void)
{
    int m = 0;
    for (Op op : ALL_OPS) {
        int c = PARAM_COUNT(op);
        if (c > m) {
            m = c;
        };
    };
    return m;
};

struct Parameter {
    ParameterMode mode;
    int value;
};

struct Instruction {
    Parameter params[MAX_PARAM_COUNT()];
    Op op;
};

std::optional<Instruction> parse_instruction(const std::vector<long>& program, size_t idx)
{
    assert(idx < program.size());

    Instruction inst;

    const int opcode = program[idx];
    assert(opcode > 0 && opcode <= 1e7);

    auto maybe_op = CODE_TO_OP(opcode % 100);
    assert(maybe_op);
    inst.op = *maybe_op;

    auto int_to_mode = [](int i) {
        if (i == 0) {
            return ParameterMode::Position;
        }
        assert(i == 1);
        return ParameterMode::Immediate;
    };

    const int pcount = PARAM_COUNT(inst.op);

    for (int i = 0; i < pcount; i++) {
        const int accum = static_cast<int>(std::pow(10, i + 2));
        inst.params[i].mode = int_to_mode((opcode / accum) % 10);
        inst.params[i].value = program[idx + i + 1];
    }

    return inst;
};

std::ostream& operator<<(std::ostream& os, const Instruction& inst)
{
    switch (inst.op) {
        case Op::Addition:
            os << "CODE: 1 (ADDITION)" << std::endl;
            break;
        case Op::Multiplication:
            os << "CODE: 2 (MULITPLICATION)" << std::endl;
            break;
        case Op::Input:
            os << "CODE: 3 (TERMINAL INPUT)" << std::endl;
            break;
        case Op::Output:
            os << "CODE: 4 (TERMINAL OUTPUT)" << std::endl;
            break;
        case Op::Halt:
            os << "CODE: 99 (HALT)" << std::endl;
            break;
    };

    const auto pcount = PARAM_COUNT(inst.op);
    for (auto i = 0; i < pcount; i++) {
        const auto mode = inst.params[i].mode;
        const auto value = inst.params[i].value;

        os << "PARAM " << i << " : " << value;
        switch (inst.params[i].mode) {
            case ParameterMode::Position:
                os << " (POSITION_MODE)" << std::endl;
                break;
            case ParameterMode::Immediate:
                os << " (IMMEDIATE_MODE)" << std::endl;
                break;
        };
    };

    os << std::endl;

    return os;
}

long run_program(std::vector<long> program, int input)
{
    const auto original_program_size = program.size();
    long pc = 0;
    bool finished = false;
    bool input_instruction_encountered = false;
    bool increment_pc_by_par_count;

    while (!finished) {
        increment_pc_by_par_count = true;

        assert(program.size() == original_program_size);
        assert(pc < program.size());

        const auto maybe_inst = parse_instruction(program, pc);
        assert(maybe_inst);
        const Instruction inst = *maybe_inst;

        auto extract = [&](ParameterMode mode, long val) {
            if (mode == ParameterMode::Immediate) {
                return val;
            }
            else {
                return program[val];
            };
        };

        if (inst.op == Op::Addition || inst.op == Op::Multiplication) {
            const long x = extract(inst.params[0].mode, inst.params[0].value);
            const long y = extract(inst.params[1].mode, inst.params[1].value);
            const long out = inst.params[2].value;

            assert(inst.params[2].mode == ParameterMode::Position);

            if (inst.op == Op::Addition) {
                program[out] = x + y;
            }
            else {
                assert(inst.op == Op::Multiplication);
                program[out] = x * y;
            }
        }
        else if (inst.op == Op::Input) {
            assert(!input_instruction_encountered);
            input_instruction_encountered = true;
            const long addr = inst.params[0].value;
            assert(inst.params[0].mode == ParameterMode::Position);

            program[addr] = input;
        }
        else if (inst.op == Op::Output) {
            const long value_to_output = extract(inst.params[0].mode, inst.params[0].value);
            std::cout << value_to_output << std::endl;
        }
        else if (inst.op == Op::Halt) {
            finished = true;
        }
        else if (inst.op == Op::JumpIfTrue) {
            const long x = extract(inst.params[0].mode, inst.params[0].value);
            const long y = extract(inst.params[1].mode, inst.params[1].value);

            if (x != 0) {
                pc = y;
                increment_pc_by_par_count = false;
            }
        }
        else if (inst.op == Op::JumpIfFalse) {
            const long x = extract(inst.params[0].mode, inst.params[0].value);
            const long y = extract(inst.params[1].mode, inst.params[1].value);

            if (x == 0) {
                pc = y;
                increment_pc_by_par_count = false;
            }
        }
        else if (inst.op == Op::LessThan) {
            const long x = extract(inst.params[0].mode, inst.params[0].value);
            const long y = extract(inst.params[1].mode, inst.params[1].value);
            const long out = inst.params[2].value;

            program[out] = x < y ? 1 : 0;
        }
        else if (inst.op == Op::Equals) {
            const long x = extract(inst.params[0].mode, inst.params[0].value);
            const long y = extract(inst.params[1].mode, inst.params[1].value);
            const long out = inst.params[2].value;

            program[out] = x == y ? 1 : 0;
        }
        else {
            assert(false && "Invalid opcode encountered");
        }

        if (increment_pc_by_par_count) {
            pc += PARAM_COUNT(inst.op) + 1;
        }
    }

    return 0;
}

int main(void)
{
    std::vector<long> program;

    {  // load program from file
        program.reserve(1000);
        std::ifstream infile("../inputs/5.txt");

        std::string istr;

        while (std::getline(infile, istr, ',')) {
            program.push_back(std::stoi(istr));
        }
    }

    run_program(program, 5);

    return 0;
}
