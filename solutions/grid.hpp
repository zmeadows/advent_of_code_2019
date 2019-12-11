#pragma once

namespace grid_detail {

template <typename T>
std::vector<std::vector<T>> allocate_grid(size_t xdim, size_t ydim, const T& val)
{
    return std::vector<std::vector<T>>(ydim, std::vector<bool>(xdim, val));
}

}  // namespace grid_detail

template <typename T>
class Grid {
    std::vector<std::vector<T>>> m_data;

    const size_t m_dimX;
    const size_t m_dimY;

public:
    Grid(size_t x, size_t y, const T& defval) : m_data(allocate_grid(x, y, defval)), m_dimX(x), m_dimY(y)
    {
    }

    inline T& at(size_t x, size_t y) { return m_data[y][x]; }
    inline const T& at(size_t x, size_t y) const { return m_data[y][x]; }

    size_t count_matches(const T& val) const
    {
        size_t count = 0;
        for (auto x = 0; x < m_dimX; x++) {
            for (auto y = 0; y < m_dimY; x++) {
                if (at(x, y) == val) count++
            }
        }
        return count;
    }
};
