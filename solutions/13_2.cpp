#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <utility>

#include "intcode.hpp"

class ArcadeCabinet {
    IntCodeVM m_computer;

    enum class TileState { Empty, Wall, Block, Paddle, Ball };
    std::map<std::pair<int, int>, TileState> m_screen;

    int m_xmax, m_xmin, m_ymin, m_ymax;
    int m_score;

    void set_screen_tile(int x, int y, TileState state)
    {
        m_screen[{x, y}] = state;
        m_xmax = std::max(m_xmax, x);
        m_xmin = std::min(m_xmin, x);
        m_ymax = std::max(m_ymax, x);
        m_ymin = std::min(m_ymin, x);
    }

    void clear_screen(void)
    {
        m_screen.clear();
        m_xmax = std::numeric_limits<int>::min();
        m_xmin = std::numeric_limits<int>::max();
        m_ymax = std::numeric_limits<int>::min();
        m_ymin = std::numeric_limits<int>::max();
    }

public:
    ArcadeCabinet(const char* filepath) : m_computer(filepath)
    {
        m_computer.write_memory(0, 2);
    }

    void tick(int input)
    {
        clear_screen();
        m_computer.set_input(input);

        while (true) {
            auto output = m_computer.continue_execution();
            if (!output) {
                assert(m_computer.get_state() != IntCodeVM::State::Halted);
                assert(m_computer.get_state() == IntCodeVM::State::AwaitingInput);
                break;
            }
            const int x = *output;
            const int y = *m_computer.continue_execution();
            const int code = *m_computer.continue_execution();

            std::cout << x << " " << y << " " << code << std::endl;

            if (x == -1 && y == 0) {
                m_score = code;
                break;
            }
            else {
                switch (code) {
                    case 0:
                        set_screen_tile(x, y, TileState::Empty);
                        break;
                    case 1:
                        set_screen_tile(x, y, TileState::Wall);
                        break;
                    case 2:
                        set_screen_tile(x, y, TileState::Block);
                        break;
                    case 3:
                        set_screen_tile(x, y, TileState::Paddle);
                        break;
                    case 4:
                        set_screen_tile(x, y, TileState::Ball);
                        break;
                    default:
                        assert(false);
                }
            }
        }

        for (int y = m_ymin; y < m_ymax; y++) {
            for (int x = m_xmin; x < m_xmax; x++) {
                switch (m_screen[{x, y}]) {
                    case TileState::Empty:
                        std::cout << " ";
                        break;
                    case TileState::Wall:
                        std::cout << "X";
                        break;
                    case TileState::Block:
                        std::cout << "#";
                        break;
                    case TileState::Paddle:
                        std::cout << "^";
                        break;
                    case TileState::Ball:
                        std::cout << "0";
                        break;
                };
            }
            std::cout << std::endl;
        }
    }
};

int main(void)
{
    ArcadeCabinet arcade("../inputs/13.txt");
    arcade.tick(0);

    int user_input_code;
    while (true) {
        std::cin >> user_input_code;

        std::cout << "user input was" << user_input_code << std::endl;

        if (user_input_code == 1) {
            arcade.tick(-1);
        }
        else if (user_input_code == 2) {
            arcade.tick(0);
        }
        else if (user_input_code == 3) {
            arcade.tick(+1);
        }
        else {
            std::cerr << "wrong input" << std::endl;
        }
    }

    return 0;
};
