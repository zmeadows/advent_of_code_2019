#include <assert.h>

#include <fstream>
#include <iostream>
#include <vector>

long run_program(std::vector<long> program, long in1, long in2)
{
    program[1] = in1;
    program[2] = in2;

    long pc = 0;
    bool finished = false;
    while (!finished) {
        const long opcode = program[pc];

        if (opcode == 1 || opcode == 2) {
            assert(pc + 3 < program.size());

            const long in1 = program[pc + 1];
            const long in2 = program[pc + 2];
            const long out = program[pc + 3];

            assert(in1 >= 0 && in1 < program.size());
            assert(in2 >= 0 && in2 < program.size());
            assert(out >= 0 && out < program.size());

            const long x = program[in1];
            const long y = program[in2];

            if (opcode == 1) {
                program[out] = x + y;
            }
            else {
                assert(opcode == 2);
                program[out] = x * y;
            }

            pc += 4;
        }
        else if (opcode == 99) {
            finished = true;
        }
        else {
            std::cerr << "Invalid opcode encountered" << std::endl;
            exit(1);
        }
    }

    return program[0];
}

int main(void)
{
    std::vector<long> program;

    {  // load program from file
        program.reserve(1000);
        std::ifstream infile("../inputs/2.txt");

        std::string istr;

        while (std::getline(infile, istr, ',')) {
            program.push_back(std::stoi(istr));
        }
    }

    bool solution_found = false;

    for (long i = 0; !solution_found && i <= 99; i++) {
        for (long j = 0; !solution_found && j <= 99; j++) {
            if (run_program(program, i, j) == 19690720) {
                std::cout << 100 * i + j << std::endl;
                solution_found = true;
            }
        }
    }

    return 0;
}
