#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

struct MoonAxis {
    static constexpr size_t MOON_COUNT = 4;

    std::array<int64_t, MOON_COUNT> pos;
    std::array<int64_t, MOON_COUNT> vel;
    const std::array<int64_t, MOON_COUNT> pos_original;
    const std::array<int64_t, MOON_COUNT> vel_original;

    size_t steps_taken;

    MoonAxis(std::array<int64_t, MOON_COUNT> p)
        : pos(p), vel({0, 0, 0, 0}), pos_original(p), vel_original(vel), steps_taken(0)
    {
    }
};

template <typename T>
T gcd(T a, T b)
{
    for (;;) {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

template <typename T>
T lcm(T a, T b)
{
    T temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
}

void update(MoonAxis& axis)
{
    auto& pos = axis.pos;
    auto& vel = axis.vel;

    for (auto i = 0; i < MoonAxis::MOON_COUNT; i++) {
        for (auto j = i + 1; j < MoonAxis::MOON_COUNT; j++) {
            if (pos[i] > pos[j]) {
                vel[i]--;
                vel[j]++;
            }
            else if (pos[i] < pos[j]) {
                vel[i]++;
                vel[j]--;
            }
        }

        pos[i] += vel[i];
    }

    axis.steps_taken++;
}

int main(void)
{
    auto start_time = std::chrono::steady_clock::now();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie();

    std::array<MoonAxis, 3> moon_axes = {MoonAxis({3, 5, -10, 8}), MoonAxis({15, -1, 8, 4}),
                                         MoonAxis({8, -2, 2, -5})};

    std::vector<uint64_t> periods;
    for (size_t i = 0; i < 3; i++) {
        auto& axis = moon_axes[i];
        while (true) {
            update(axis);
            if (axis.pos == axis.pos_original && axis.vel == axis.vel_original) {
                periods.push_back(axis.steps_taken);
                break;
            }
        }
    }

    std::cout << std::accumulate(periods.begin(), periods.end(), (uint64_t)1, lcm<uint64_t>)
              << std::endl;

    auto end_time = std::chrono::steady_clock::now();

    std::cout << "part two computation time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;
}
