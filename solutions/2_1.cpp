#include <assert.h>

#include <fstream>
#include <iostream>
#include <vector>

int main(void)
{
    std::vector<int> program;

    {  // load program from file
        program.reserve(1000);
        std::ifstream infile("../inputs/2.txt");

        std::string istr;

        while (std::getline(infile, istr, ',')) {
            program.push_back(std::stoi(istr));
        }
    }

    program[1] = 12;
    program[2] = 2;

    int pc = 0;
    bool finished = false;
    while (!finished) {
        const int opcode = program[pc];

        if (opcode == 1 || opcode == 2) {
            assert(pc + 3 < program.size());

            const int in1 = program[pc + 1];
            const int in2 = program[pc + 2];
            const int out = program[pc + 3];

            assert(in1 >= 0 && in1 < program.size());
            assert(in2 >= 0 && in2 < program.size());
            assert(out >= 0 && out < program.size());

            const int x = program[in1];
            const int y = program[in2];

            if (opcode == 1) {
                program[out] = x + y;
            }
            else {
                assert(opcode == 2);
                program[out] = x * y;
            }
        }
        else if (opcode == 99) {
            finished = true;
        }
        else {
            std::cerr << "Invalid opcode encountered" << std::endl;
            exit(1);
        }

        pc += 4;
    }

    std::cout << program[0] << std::endl;

    return 0;
}
