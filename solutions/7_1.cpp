#include <assert.h>

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

// #define DEBUG_COMPUTER

enum class ParameterMode { Position, Immediate };

// NOTE: remember to update ALL_OPS/CODE_TO_OP/PARAM_COUNT if you add new op codes!!!
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
    long value;
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

long run_program(std::vector<long> program, int phase, int input)
{
    const auto original_program_size = program.size();
    long pc = 0;
    bool finished = false;
    int input_instruction_encountered = 0;

#ifdef DEBUG_COMPUTER
    std::cout << std::endl;
    std::cout << "Running fresh program with phase: " << phase << std::endl
              << "                           input: " << input << std::endl;
    std::cout << std::endl;
#endif

    auto print_program = [&program](void) {
#ifdef DEBUG_COMPUTER
        for (auto i = 0; i < program.size(); i++) {
            std::cout << "[" << i << "] " << program[i] << " ";
        }
        std::cout << std::endl << std::endl;
#endif
    };

    while (!finished) {
        print_program();

        bool increment_pc_by_par_count = true;

        // ensure we aren't accidentally modifying program length
        assert(program.size() == original_program_size);
        // ensure we aren't moving past end of program
        assert(pc < program.size());

        const auto maybe_inst = parse_instruction(program, pc);
        assert(maybe_inst);  // instruction parsing should never fail
        const Instruction inst = *maybe_inst;

        auto extract = [&program](Parameter param) {
            if (param.mode == ParameterMode::Immediate) {
                return param.value;
            }
            else {
                return program[param.value];
            };
        };

        if (inst.op == Op::Addition || inst.op == Op::Multiplication) {
            const long x = extract(inst.params[0]);
            const long y = extract(inst.params[1]);
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
            assert(input_instruction_encountered < 2);
            input_instruction_encountered++;
            const long addr = inst.params[0].value;
            assert(inst.params[0].mode == ParameterMode::Position);

            if (input_instruction_encountered == 1) {
                program[addr] = phase;
            }
            else {
                assert(input_instruction_encountered == 2);
                program[addr] = input;
            }
        }
        else if (inst.op == Op::Output) {
            const long value_to_output = extract(inst.params[0]);
            return value_to_output;
        }
        else if (inst.op == Op::Halt) {
            assert(false);
            finished = true;
        }
        else if (inst.op == Op::JumpIfTrue) {
            const long x = extract(inst.params[0]);
            const long y = extract(inst.params[1]);

            if (x != 0) {
                pc = y;
                increment_pc_by_par_count = false;
            }
        }
        else if (inst.op == Op::JumpIfFalse) {
            const long x = extract(inst.params[0]);
            const long y = extract(inst.params[1]);

            if (x == 0) {
                pc = y;
                increment_pc_by_par_count = false;
            }
        }
        else if (inst.op == Op::LessThan) {
            const long x = extract(inst.params[0]);
            const long y = extract(inst.params[1]);
            const long out = inst.params[2].value;

            program[out] = x < y ? 1 : 0;
        }
        else if (inst.op == Op::Equals) {
            const long x = extract(inst.params[0]);
            const long y = extract(inst.params[1]);
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

void run_over_permutations(const std::vector<long>& program, std::array<int, 5> phases, int L,
                           long& max_output)
{
    if (L == phases.size() - 1) {
        long input = 0;
        for (auto amp_index = 0; amp_index < 5; amp_index++) {
            input = run_program(program, phases[amp_index], input);
        }
        max_output = input > max_output ? input : max_output;
    }
    else {
        for (size_t R = 0; R < phases.size(); R++) {
            std::swap(phases[L], phases[R]);
            run_over_permutations(program, phases, L + 1, max_output);
            std::swap(phases[L], phases[R]);
        }
    }
}

int main(void)
{
    std::vector<long> program;

    {  // load program from file
        program.reserve(256);
        std::ifstream infile("../inputs/7.txt");

        std::string istr;

        while (std::getline(infile, istr, ',')) {
            program.push_back(std::stoi(istr));
        }
    }

    const auto program_copy = program;

    long max_output = -1;
    run_over_permutations(program, {0, 1, 2, 3, 4}, 0, max_output);
    std::cout << max_output << std::endl;

    // int input = 0;

    // for (auto amp_index = 0; amp_index < 5; amp_index++) {
    //     input = run_program(program_copy, phases[amp_index], input);
    // }

    // max_output = input > max_output ? input : max_output;

    // std::cout << max_output << std::endl;

    return 0;
}
