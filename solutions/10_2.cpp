#include <assert.h>
#include <math.h> /* fabs */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

static int gcd(int a, int b)
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

    std::cout << "Asteroid Map:" << std::endl;
    for (auto y = 0; y < ydim; y++) {
        for (auto x = 0; x < xdim; x++) {
            std::cout << (occupancy[y][x] ? '#' : '.');
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return occupancy;
}

class AsteroidMap {
    std::vector<std::vector<bool>> m_occupancy;
    const int m_xdim;
    const int m_ydim;

public:
    AsteroidMap(const char* filepath)
        : m_occupancy(read_occupancy_from_file(filepath)),
          m_xdim(!m_occupancy.empty() ? m_occupancy[0].size() : 0),
          m_ydim(m_occupancy.size())
    {
    }

    std::vector<std::vector<bool>> allocate_fov_buffer(void) const
    {
        return std::vector<std::vector<bool>>(m_ydim, std::vector<bool>(m_xdim, true));
    }

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

    inline void vaporize_at(int x, int y) { m_occupancy[y][x] = false; }
};

// pass fov_buffer in as argument to avoid un-necessary re-allocations
static void compute_asteroid_fov(const AsteroidMap& asteroids,
                                 std::vector<std::vector<bool>>& fov_buffer, int x0, int y0)
{
    const auto XDIM = asteroids.dimension_x();
    const auto YDIM = asteroids.dimension_y();

    if (fov_buffer.size() != YDIM || fov_buffer[0].size() != XDIM) {
        fov_buffer.clear();
        fov_buffer = asteroids.allocate_fov_buffer();
    }

    auto is_point_on_map = [&](int x, int y) { return (x >= 0 && x < XDIM && y >= 0 && y < YDIM); };

    assert(is_point_on_map(x0, y0));

    // set all asteroids to true, all empty spaces to false
    // since we only care about what asteroids we can see
    for (auto y = 0; y < YDIM; y++) {
        for (auto x = 0; x < XDIM; x++) {
            fov_buffer[y][x] = asteroids.is_point_occupied(x, y);
        }
    }

    auto block_point = [&](int x_block, int y_block) { fov_buffer[y_block][x_block] = false; };

    // for our purposes here, don't want to count the asteroid itself
    // for a typical FOV algorithm this wouldn't be the case
    block_point(x0, y0);

    enum class Direction { Up, Down, Left, Right };
    int dx = 1;
    int dy = 1;
    Direction dir = Direction::Down;
    int loop_count = 1;

    int squares_visited = 0;

    // visit all squares except one where we are computing visibility
    while (squares_visited < XDIM * YDIM - 1) {
        const int x = x0 + dx;
        const int y = y0 + dy;

        if (is_point_on_map(x, y)) {
            squares_visited++;

            const bool point_is_an_asteroid = asteroids.is_point_occupied(x, y);
            const bool point_currently_visible = fov_buffer[y][x];

            if (point_currently_visible && point_is_an_asteroid) {
                // found a non-blocked asteroid, so block all further asteroids in same line of sight.

                const int den = gcd(std::abs(dx), std::abs(dy));
                const int dx_block = dx / den;
                const int dy_block = dy / den;
                int x2 = x + dx_block;
                int y2 = y + dy_block;

                while (is_point_on_map(x2, y2)) {
                    block_point(x2, y2);
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

struct BestMonitoringStation {
    int x_loc = -1;
    int y_loc = -1;
    int view_count = -1;
    std::vector<std::vector<bool>> fov;
};

BestMonitoringStation find_best_monitoring_station(const AsteroidMap& asteroids)
{
    auto fov = asteroids.allocate_fov_buffer();

    const auto XDIM = asteroids.dimension_x();
    const auto YDIM = asteroids.dimension_y();

    BestMonitoringStation best;
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

                if (view_count > best.view_count) {
                    best.view_count = view_count;
                    best.x_loc = x0;
                    best.y_loc = y0;
                    std::swap(best.fov, fov);
                }
            }
        }
    }

    std::cout << "Best monitoring station sees " << best.view_count << " asteroids @ " << best.x_loc
              << "," << best.y_loc << std::endl;

    return best;
}

std::vector<std::pair<int, int>> convert_fov_to_positions(const std::vector<std::vector<bool>>& fov)
{
    std::vector<std::pair<int, int>> result;
    result.reserve(fov.size());

    const auto ydim = fov.size();
    assert(ydim > 0);
    const auto xdim = fov[0].size();

    for (auto y = 0; y < ydim; y++) {
        for (auto x = 0; x < xdim; x++) {
            if (fov[y][x]) result.emplace_back(x, y);
        }
    }

    return result;
}

float compute_asteroid_angle_relative_to_station(std::pair<int, int> offset)
{
    const float dx = static_cast<float>(offset.first);
    const float dy = static_cast<float>(offset.second);
    assert(!(dx == 0 && dy == 0));

    const float a0 = std::atan2(1.f, 0.f);
    const float a = std::atan2(dy, dx);
    const float delta = a - a0;

    if (delta < 0) {
        return -1.f * delta;
    }
    else if (delta > 0) {
        return 2 * M_PI - delta;
    }
    else {
        return delta;
    }
}

inline bool compare_asteroid_angles_relative_to_station(std::pair<int, int> a, std::pair<int, int> b)
{
    return compute_asteroid_angle_relative_to_station(a) < compute_asteroid_angle_relative_to_station(b);
}

// std::pair<int, int> find_coordinates_of_two_hundreth_vaporized_asteroid_from_best_monitoring_station(
//     const AsteroidMap& _asteroids)
// {
//     AsteroidMap asteroids(_asteroids);
//
//     const int XDIM = asteroids.dimension_x();
//     const int YDIM = asteroids.dimension_y();
//
//     BestMonitoringStation best = find_best_monitoring_location(asteroids);
//
//     const int x0 = best.x_loc;
//     const int y0 = best.y_loc;
//
//     int vaped_nation_count = 0;
//
//     std::vector<std::pair<int, int>> vape_coords;
//     vape_coords.reserve(best.view_count);
//
//     while (true) {
//         vape_coords.clear();
//
//         auto vis = find_visible_asteroids(asteroids, best.fov);
//
//         for (auto x = 0; x < XDIM; x++) {
//             for (auto y = 0; y < YDIM; y++) {
//                 if (vis[y][x]) {
//                     vape_coords.push_back({x - x0, y - y0});
//                 }
//             }
//         }
//
//         sort(vape_coords.begin(), vape_coords.end(), compare_asteroid_angle);
//
//         for (auto [x_vis, y_vis] : vape_coords) {
//             asteroids.destroy_asteroid_at(x0 + x_vis, y0 + y_vis);
//             vaped_nation_count++;
//             if (vaped_nation_count == 200) {
//                 return {x0 + x_vis, y0 + y_vis};
//             }
//         }
//
//         compute_asteroid_fov(asteroids, best.fov, x0, y0);
//     }
//
//     return {-1, -1};
// }

int main(void)
{
    auto start_time = std::chrono::steady_clock::now();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie();

    AsteroidMap asteroids("../inputs/10.txt");
    auto best_station = find_best_monitoring_station(asteroids);
    auto end_time = std::chrono::steady_clock::now();

    std::cout << "part one computation time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;

    start_time = std::chrono::steady_clock::now();

    std::vector<std::pair<int, int>> visible_asteroid_locations =
        convert_fov_to_positions(best_station.fov);

    // convert the locations into offsets relative to station
    for (auto& [x, y] : visible_asteroid_locations) {
        x -= best_station.x_loc;
        y -= best_station.y_loc;
        y *= -1;  // dealing with computer graphics style coordinate system where y increases moving
                  // downard
    }

    std::sort(visible_asteroid_locations.begin(), visible_asteroid_locations.end(),
              compare_asteroid_angles_relative_to_station);

    for (auto& [x, y] : visible_asteroid_locations) {
        std::cout << x << " " << y << std::endl;
    }

    std::cout << best_station.x_loc + visible_asteroid_locations[199].first << std::endl;
    std::cout << best_station.y_loc - visible_asteroid_locations[199].second << std::endl;

    end_time = std::chrono::steady_clock::now();

    std::cout << "part two computation time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()
              << "us" << std::endl;

    return 0;
}
