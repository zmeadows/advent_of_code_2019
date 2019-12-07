#include <assert.h>

#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
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

std::optional<Instruction> parse_instruction(const std::vector<long>& program, size_t pc)
{
    assert(pc < program.size());

    Instruction inst;

    const int opcode = program[pc];
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
        inst.params[i].value = program[pc + i + 1];
    }

    return inst;
};

class Amplifier {
    std::vector<long> m_state;
    const long m_phase;

    size_t m_pc;  // program counter

    bool m_phase_consumed;  // 'true' after phase has been used as input and the amplifier is ready to
                            // use the provided input from the previous amp (or use the initial '0' input
                            // for the first iteration of the first amplifier)
                            //
    bool m_halted;          // 'true' if HALT op code encountered

public:
    Amplifier(const std::vector<long>& init_state, long _phase)
        : m_state(init_state), m_phase(_phase), m_pc(0), m_phase_consumed(false), m_halted(false)
    {
    }

    inline bool is_halted(void) const { return m_halted; }

    // input will be empty when previous amp halted
    std::optional<long> begin_or_resume_execution(std::optional<long> input)
    {
        if (m_halted) {
            assert(false);  // should never try to run a halted amplifier again
            return {};
        }

        const auto original_program_size = m_state.size();
        bool input_instruction_encountered = false;
        std::optional<long> output;

#ifdef DEBUG_COMPUTER
        std::cout << std::endl;
        std::cout << "Running fresh program with phase: " << phase << std::endl
                  << "                           input: " << input << std::endl;
        std::cout << std::endl;
#endif

        auto print_program = [this](void) {
#ifdef DEBUG_COMPUTER
            for (auto i = 0; i < this->m_state.size(); i++) {
                std::cout << "[" << i << "] " << this->m_state[i] << " ";
            }
            std::cout << std::endl << std::endl;
#endif
        };

        while (true) {
            // each iteration of this while loop == 1 instruction processed
            print_program();

            bool increment_pc_by_par_count = true;

            // ensure we aren't accidentally modifying program length
            assert(m_state.size() == original_program_size);
            // ensure we aren't moving past end of program
            assert(m_pc < m_state.size());

            const auto maybe_inst = parse_instruction(m_state, m_pc);
            assert(maybe_inst);  // instruction parsing should never fail
            const Instruction inst = *maybe_inst;

            // account for position vs. immediate mode when extract instruction parameters from state
            auto extract = [this](Parameter param) {
                if (param.mode == ParameterMode::Immediate) {
                    return param.value;
                }
                else {
                    return this->m_state[param.value];
                };
            };

            if (inst.op == Op::Addition || inst.op == Op::Multiplication) {
                const long x = extract(inst.params[0]);
                const long y = extract(inst.params[1]);
                const long out = inst.params[2].value;

                assert(inst.params[2].mode == ParameterMode::Position);

                if (inst.op == Op::Addition) {
                    m_state[out] = x + y;
                }
                else {
                    assert(inst.op == Op::Multiplication);
                    m_state[out] = x * y;
                }
            }
            else if (inst.op == Op::Input) {
                assert(!input_instruction_encountered);

                assert(inst.params[0].mode == ParameterMode::Position);
                const long addr = inst.params[0].value;

                if (!m_phase_consumed) {
                    m_state[addr] = m_phase;
                    m_phase_consumed = true;
                }
                else {
                    assert(!input_instruction_encountered);
                    input_instruction_encountered = true;
                    assert(input);  // input should never be empty when input instruction encountered
                    m_state[addr] = *input;
                }
            }
            else if (inst.op == Op::Output) {
                output = extract(inst.params[0]);
            }
            else if (inst.op == Op::Halt) {
                m_halted = true;
                return {};
            }
            else if (inst.op == Op::JumpIfTrue) {
                const long x = extract(inst.params[0]);
                const long y = extract(inst.params[1]);

                if (x != 0) {
                    m_pc = y;
                    increment_pc_by_par_count = false;
                }
            }
            else if (inst.op == Op::JumpIfFalse) {
                const long x = extract(inst.params[0]);
                const long y = extract(inst.params[1]);

                if (x == 0) {
                    m_pc = y;
                    increment_pc_by_par_count = false;
                }
            }
            else if (inst.op == Op::LessThan) {
                const long x = extract(inst.params[0]);
                const long y = extract(inst.params[1]);
                const long out = inst.params[2].value;

                m_state[out] = x < y ? 1 : 0;
            }
            else if (inst.op == Op::Equals) {
                const long x = extract(inst.params[0]);
                const long y = extract(inst.params[1]);
                const long out = inst.params[2].value;

                m_state[out] = x == y ? 1 : 0;
            }
            else {
                assert(false && "Invalid opcode encountered");
            }

            if (increment_pc_by_par_count) {
                m_pc += PARAM_COUNT(inst.op) + 1;
            }

            if (output) return output;
        }
    }
};

long run_amplifier_chain(const std::vector<long>& program, const std::array<long, 5>& phases)
{
    std::array<Amplifier, 5> amps = {
        Amplifier(program, phases[0]), Amplifier(program, phases[1]), Amplifier(program, phases[2]),
        Amplifier(program, phases[3]), Amplifier(program, phases[4]),
    };

    size_t current_amp_index = 0;
    std::optional<long> last_amplifier_E_output = {};
    std::optional<long> next_amplifier_input = 0;
    std::array<std::optional<long>, 5> output_history;
    bool all_amps_halted = false;

    while (!all_amps_halted) {
        // output of current amp is the input for the next amp
        next_amplifier_input = amps[current_amp_index].begin_or_resume_execution(next_amplifier_input);
        output_history[current_amp_index] = next_amplifier_input;

        if (current_amp_index == 4 && next_amplifier_input) {
            last_amplifier_E_output = next_amplifier_input;
        }

        if (current_amp_index == 4 && !next_amplifier_input) {
            // double check that all amps actually halted on same cycle
            for (auto o : output_history) assert(!o);
            for (const Amplifier& a : amps) assert(a.is_halted());

            // double check that we actually got at least one output from amplifier E
            assert(last_amplifier_E_output);
            return *last_amplifier_E_output;
        }

        current_amp_index = (current_amp_index + 1) % 5;
    }

    assert(false);
    return 0;
}

void test_all_phase_permutations(const std::vector<long>& initial_program_state,
                                 std::array<long, 5> phases, int L, long& max_output)
{
    if (L == phases.size() - 1) {
        long chain_output = run_amplifier_chain(initial_program_state, phases);
        max_output = std::max(chain_output, max_output);
    }
    else {
        for (size_t R = 0; R < phases.size(); R++) {
            std::swap(phases[L], phases[R]);
            test_all_phase_permutations(initial_program_state, phases, L + 1, max_output);
            std::swap(phases[L], phases[R]);
        }
    }
}

long find_max_thruster_signal(const std::vector<long>& initial_program_state)
{
    long max_thruster_signal = std::numeric_limits<long>::min();
    test_all_phase_permutations(initial_program_state, {5, 6, 7, 8, 9}, 0, max_thruster_signal);
    return max_thruster_signal;
}

int main(void)
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie();

    auto start_time = std::chrono::steady_clock::now();

    std::vector<long> program;

    {  // load program from file
        program.reserve(1028);
        std::ifstream infile("../inputs/7.txt");

        std::string istr;

        while (std::getline(infile, istr, ',')) {
            program.push_back(std::stoi(istr));
        }
    }

    std::cout << find_max_thruster_signal(program) << std::endl;

    const auto end_time = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;

    return 0;
}
