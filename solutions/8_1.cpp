#include <assert.h>

#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

int main(void)
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie();

    auto start_time = std::chrono::steady_clock::now();

    constexpr int pixels_in_layer = 25 * 6;

    std::ifstream infile("../inputs/8.txt");

    std::vector<int> layer;
    std::string buffer;
    std::vector<std::vector<int>> image;
    char c;

    while (infile.get(c)) {
        if (layer.size() == pixels_in_layer) {
            image.emplace_back(std::move(layer));
            layer.clear();
            layer.reserve(pixels_in_layer);
        }

        const int pixel = c - '0';
        if (!(pixel >= 0 && pixel <= 2)) break;
        layer.push_back(pixel);
    }

    std::cout << "read " << image.size() << " layers" << std::endl;

    int min_zero_digits = std::numeric_limits<int>::max();
    std::optional<int> result;
    for (const std::vector<int>& layer : image) {
        int zero_digits = 0;
        int one_digits = 0;
        int two_digits = 0;

        for (const int digit : layer) {
            assert(digit >= 0 && digit <= 2);
            if (digit == 0) zero_digits++;
            if (digit == 1) one_digits++;
            if (digit == 2) two_digits++;
        }

        if (zero_digits < min_zero_digits) {
            min_zero_digits = zero_digits;
            result = one_digits * two_digits;
        }
    }

    const auto end_time = std::chrono::steady_clock::now();

    assert(result);
    std::cout << "answer = " << *result << std::endl;

    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;

    return 0;
}
