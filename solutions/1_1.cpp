#include <fstream>
#include <iostream>

int main(void)
{
    std::ifstream infile("../inputs/1.txt");

    int fuel = 0;
    int mass;
    while (infile >> mass) {
        fuel += mass / 3 - 2;
    }
    std::cout << fuel << std::endl;
}
