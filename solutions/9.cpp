#include "intcode.hpp"

int main(void)
{
    IntCodeVM vm("../inputs/9.txt");
    vm.run_until_halt_with_single_input(2);
    return 0;
}
