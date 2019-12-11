#include <map>

#include "intcode.hpp"

enum class Color { White, Black };
enum class Direction { Up, Down, Left, Right };
using Position = std::pair<int, int>;

class HullPaintingRobot {
    IntCodeVM m_computer;
    std::map<Position, Color> m_hull;
    Position m_position;
    Direction m_direction;
    size_t m_panels_painted;

    inline Color color_at(Position pos)
    {
        auto it = m_hull.find(pos);
        return it == m_hull.end() ? Color::Black : it->second;
    }

    inline void paint_panel(Position pos, Color col)
    {
        auto it = m_hull.find(pos);
        if (it == m_hull.end()) {
            m_hull[pos] = col;
            m_panels_painted++;
        }
        else {
            it->second = col;
        }
    }

public:
    HullPaintingRobot(const char* program_filepath, bool start_on_white)
        : m_computer(program_filepath),
          m_position({0, 0}),
          m_direction(Direction::Up),
          m_panels_painted(0)
    {
        if (start_on_white) {
            m_hull[m_position] = Color::White;
        }
    }

    void print_hull(void)
    {
        // first determine min/max extent that robot painted on hull
        int x_min = std::numeric_limits<int>::max();
        int x_max = std::numeric_limits<int>::min();
        int y_min = std::numeric_limits<int>::max();
        int y_max = std::numeric_limits<int>::min();

        for (const auto& [pos, _] : m_hull) {
            x_min = std::min(x_min, pos.first);
            x_max = std::max(x_max, pos.first);
            y_min = std::min(y_min, pos.second);
            y_max = std::max(y_max, pos.second);
        }

        // starting from the top, print the rows of painted squares
        for (auto y = y_max; y >= y_min; y--) {
            for (auto x = x_min; x <= x_max; x++) {
                auto it = m_hull.find({x, y});
                if (it != m_hull.end()) {
                    std::cout << (it->second == Color::White ? "*" : " ");
                }
                else {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
    }

    size_t paint(void)
    {
        assert(m_panels_painted == 0);

        bool still_painting = true;
        while (still_painting) {
            Color color_underneath_robot = color_at(m_position);

            auto output = m_computer.continue_execution(color_underneath_robot == Color::Black ? 0 : 1);

            if (output) {
                paint_panel(m_position, *output == 0 ? Color::Black : Color::White);
                output = m_computer.continue_execution({});
                assert(output);  // robot should always return direction to turn

                switch (*output) {
                    case 0:  // turn left 90 degrees
                        switch (m_direction) {
                            case Direction::Up:
                                m_direction = Direction::Left;
                                break;
                            case Direction::Down:
                                m_direction = Direction::Right;
                                break;
                            case Direction::Left:
                                m_direction = Direction::Down;
                                break;
                            case Direction::Right:
                                m_direction = Direction::Up;
                                break;
                        }
                        break;
                    case 1:  // turn right 90 degrees
                        switch (m_direction) {
                            case Direction::Up:
                                m_direction = Direction::Right;
                                break;
                            case Direction::Down:
                                m_direction = Direction::Left;
                                break;
                            case Direction::Left:
                                m_direction = Direction::Up;
                                break;
                            case Direction::Right:
                                m_direction = Direction::Down;
                                break;
                        }
                        break;
                    default:
                        std::cerr << "ERROR: invalid robot turn direction output encountered."
                                  << std::endl;
                        exit(EXIT_FAILURE);
                }

                // now move the robot
                switch (m_direction) {
                    case Direction::Up:
                        m_position.second++;
                        break;
                    case Direction::Down:
                        m_position.second--;
                        break;
                    case Direction::Left:
                        m_position.first--;
                        break;
                    case Direction::Right:
                        m_position.first++;
                        break;
                }
            }
            else {
                still_painting = false;
            }
        }

        assert(m_computer.is_halted());

        return m_panels_painted;
    }
};

int main(void)
{
    HullPaintingRobot part_one_robot("../inputs/11.txt", false);
    std::cout << "part one answer = " << part_one_robot.paint() << std::endl;
    HullPaintingRobot part_two_robot("../inputs/11.txt", true);
    part_two_robot.paint();
    part_two_robot.print_hull();
}
