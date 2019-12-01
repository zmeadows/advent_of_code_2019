#include <fstream>
#include <iostream>

inline int compute_fuel(int mass)
{
    int total_fuel = 0;
    int fuel = mass / 3 - 2;

    while (fuel > 0) {
        total_fuel += fuel;
        fuel = fuel / 3 - 2;
    }

    return total_fuel;
}

int main(void)
{
    std::ifstream infile("../inputs/1.txt");

    int fuel = 0;
    int mass;
    while (infile >> mass) {
        fuel += compute_fuel(mass);
    }
    std::cout << fuel << std::endl;
}
