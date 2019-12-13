#include "intcode.hpp"

int main(void)
{
    IntCodeVM vm("../inputs/13.txt");

    size_t output_count = 0;
    size_t block_count = 0;

    while (!vm.is_halted()) {
        auto output = vm.continue_execution({});
        if (output) output_count++;
        if (output_count % 3 == 0 && *output == 2) {
            block_count++;
        }
    }

    std::cout << block_count << std::endl;
};
