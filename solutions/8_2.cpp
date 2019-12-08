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

    std::array<bool, pixels_in_layer> final_image;

    for (auto i = 0; i < pixels_in_layer; i++) {
        for (const std::vector<int>& layer : image) {
            if (layer[i] == 0) {
                final_image[i] = true;
                break;
            }
            else if (layer[i] == 1) {
                final_image[i] = false;
                break;
            }
        }
    }

    const auto end_time = std::chrono::steady_clock::now();

    for (auto y = 0; y < 6; y++) {
        for (auto x = 0; x < 25; x++) {
            const auto idx = x + 25 * y;
            if (final_image[idx]) {
                std::cout << " ";
            }
            else {
                std::cout << "X";
            }
        }
        std::cout << std::endl;
    }

    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;

    return 0;
}
