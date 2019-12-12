#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

struct MoonAxis {
    static constexpr size_t MOON_COUNT = 4;

    std::array<int, MOON_COUNT> pos;
    std::array<int, MOON_COUNT> vel;
    const std::array<int, MOON_COUNT> pos_original;
    const std::array<int, MOON_COUNT> vel_original;

    size_t steps_taken;

    MoonAxis(std::array<int, MOON_COUNT> p)
        : pos(p), vel({0, 0, 0, 0}), pos_original(p), vel_original(vel), steps_taken(0)
    {
    }
};

void update(MoonAxis& axis)
{
    std::array<int, MoonAxis::MOON_COUNT>& pos = axis.pos;
    std::array<int, MoonAxis::MOON_COUNT>& vel = axis.vel;

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

    for (size_t i = 0; i < 3; i++) {
        auto& axis = moon_axes[i];
        while (axis.steps_taken < 1000) {
            update(axis);
        }
    }

    size_t total_energy = 0;
    for (auto i = 0; i < 4; i++) {
        int pot_energy = 0;
        int kin_energy = 0;
        for (auto j = 0; j < 3; j++) {
            pot_energy += std::abs(moon_axes[j].pos[i]);
            kin_energy += std::abs(moon_axes[j].vel[i]);
        }
        total_energy += pot_energy * kin_energy;
    }

    auto end_time = std::chrono::steady_clock::now();

    std::cout << total_energy << std::endl;

    std::cout << "part one computation time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;
}
