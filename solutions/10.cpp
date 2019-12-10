#include <assert.h>
#include <math.h> /* fabs */

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

int gcd(int a, int b)
{
    // Everything divides 0
    if (a == 0) return b;
    if (b == 0) return a;

    // base case
    if (a == b) return a;

    // a is greater
    if (a > b) return gcd(a - b, b);
    return gcd(a, b - a);
}

class AsteroidMap {
    std::vector<std::vector<bool>> m_occupancy;
    int m_xdim;
    int m_ydim;

    static std::vector<std::vector<bool>> read_occupancy_from_file(const char* filepath)
    {
        std::ifstream infile(filepath);
        std::string line;

        std::getline(infile, line);
        const int xdim = line.size();

        std::vector<std::vector<bool>> occupancy;

        do {
            std::vector<bool> occupancy_row;
            occupancy_row.reserve(xdim);

            for (auto ch : line) {
                occupancy_row.push_back(ch == '#');
            }

            occupancy.push_back(occupancy_row);

        } while (std::getline(infile, line));

        const int ydim = occupancy.size();

        assert(xdim > 0);
        assert(ydim > 0);

        std::cout << "Successfully read asteroid map from file: " << filepath << std::endl;
        std::cout << "Dimensions: " << xdim << " by " << ydim << std::endl;
        std::cout << std::endl;

        for (auto y = 0; y < ydim; y++) {
            for (auto x = 0; x < xdim; x++) {
                std::cout << (occupancy[y][x] ? '#' : '.');
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        return occupancy;
    }

public:
    bool is_point_occupied(int x, int y) const
    {
        if (x >= m_xdim || y >= m_ydim) {
            return false;
        }
        else {
            return m_occupancy[y][x];
        }
    }

    int dimension_x(void) const { return m_xdim; }
    int dimension_y(void) const { return m_ydim; }

    AsteroidMap(const char* filepath)
        : m_occupancy(read_occupancy_from_file(filepath)),
          m_xdim(!m_occupancy.empty() ? m_occupancy[0].size() : 0),
          m_ydim(m_occupancy.size())
    {
    }
};

void compute_asteroid_fov(const AsteroidMap& asteroids, std::vector<std::vector<bool>>& fov, int x0,
                          int y0)
{
    const auto XDIM = asteroids.dimension_x();
    const auto YDIM = asteroids.dimension_y();
    assert(fov.size() == YDIM && fov[0].size() == XDIM);

    auto is_point_on_map = [&](int x, int y) { return (x >= 0 && x < XDIM && y >= 0 && y < YDIM); };

    assert(is_point_on_map(x0, y0));

    // reset fov to all be visible, before we block in a spiral below
    for (auto y = 0; y < YDIM; y++) {
        for (auto x = 0; x < XDIM; x++) {
            fov[y][x] = true;
        }
    }

    fov[y0][x0] = false;

    enum class Direction { Up, Down, Left, Right };
    int dx = 1;
    int dy = 1;
    Direction dir = Direction::Down;
    int loop_count = 1;

    int squares_visited = 0;

    // visit all squares expect one where we are computing visibility
    while (squares_visited < XDIM * YDIM - 1) {
        const int x = x0 + dx;
        const int y = y0 + dy;

        if (is_point_on_map(x, y)) {
            squares_visited++;

            const bool point_currently_visible = is_point_on_map(x, y) && fov[y][x];

            // if point already blocked, nothing to do
            if (point_currently_visible && asteroids.is_point_occupied(x, y)) {
                // found a non-blocked asteroid. block all further asteroids in same line of sight.

                const int den = gcd(std::abs(dx), std::abs(dy));
                const int dx_block = dx / den;
                const int dy_block = dy / den;
                int x2 = x + dx_block;
                int y2 = y + dy_block;

                // std::cout << "gcd =  " << den << std::endl;
                // std::cout << "dx/dy = " << dx << " " << dy << std::endl;
                // std::cout << "dx/dy block = " << dx_block << " " << dy_block << std::endl;
                // std::cout << std::endl;

                while (is_point_on_map(x2, y2)) {
                    fov[y2][x2] = false;
                    x2 += dx_block;
                    y2 += dy_block;
                }
            }
        }

        // do expanding loops in clockwise direction
        switch (dir) {
            case Direction::Up:
                if (dy == loop_count) {
                    dir = Direction::Right;
                    dx++;
                }
                else {
                    dy++;
                }
                break;
            case Direction::Down:
                if (dy == -loop_count) {
                    dir = Direction::Left;
                    dx--;
                }
                else {
                    dy--;
                }
                break;
            case Direction::Left:
                if (dx == -loop_count) {
                    dir = Direction::Up;
                    dy++;
                }
                else {
                    dx--;
                }
                break;
            case Direction::Right:
                if (dx == loop_count) {
                    loop_count++;
                    dir = Direction::Down;
                    dy--;
                }
                else {
                    dx++;
                }
                break;
        }

        assert(!(dx == 0 && dy == 0));
    }
}

// std::vector<std::vector<bool>> compute_visibility_map(const std::vector<std::vector<bool>>&
// asteroid_map,
//                                                       int x, int y)
// {
//     auto is_asteroid_at = [&](int x, int y) { return asteroid_map[y][x]; };
//
//     if (asteroid_map[y][x] == false) {
//         return -1;
//     }
//
//     const size_t x_dim = asteroid_map[0].size();
//     const size_t y_dim = asteroid_map.size();
//
//     std::vector<float> sight_angles_blocked;
//     sight_angles_blocked.reserve(0.5 * x_dim * y_dim);
//
//     auto float_equals = [](float a, float b) { return std::fabs(a - b) < 1e-7; };
//
//     int visibility = 0;
//
//     for (auto ix = 0; ix < x_dim; ix++) {
//         for (auto iy = 0; iy < y_dim; iy++) {
//             if (ix == x && iy == y) continue;
//
//             bool blocked = false;
//             for (float angle : sight_angles_blocked) {
//                 if (float_equals(angle, dydx)) {
//                     blocked = true;
//                 }
//             }
//
//             if (!blocked) {
//                 visibility++;
//                 sight_angles_blocked.push_back(dydx);
//             }
//         }
//     }
//
//     return visibility;
// }

int main(void)
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie();

    auto start_time = std::chrono::steady_clock::now();

    AsteroidMap asteroids("../inputs/10.txt");

    const auto XDIM = asteroids.dimension_x();
    const auto YDIM = asteroids.dimension_y();

    std::vector<std::vector<bool>> fov(YDIM, std::vector<bool>(XDIM, true));

    int max_view_count = -1;
    int x_max = -1;
    int y_max = -1;
    for (auto x0 = 0; x0 < XDIM; x0++) {
        for (auto y0 = 0; y0 < YDIM; y0++) {
            if (asteroids.is_point_occupied(x0, y0)) {
                compute_asteroid_fov(asteroids, fov, x0, y0);
                int view_count = 0;
                for (auto x = 0; x < XDIM; x++) {
                    for (auto y = 0; y < YDIM; y++) {
                        if (!(x0 == x && y0 == y) && fov[y][x] && asteroids.is_point_occupied(x, y)) {
                            view_count++;
                        }
                    }
                }

                if (view_count > max_view_count) {
                    max_view_count = view_count;
                    x_max = x0;
                    y_max = y0;
                }
            }
        }
    }

    auto end_time = std::chrono::steady_clock::now();

    std::cout << "Best monitoring station sees " << max_view_count << " asteroids @ " << x_max << ","
              << y_max << std::endl;

    std::cout << "computation time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;
    return 0;
}
