#include "intcode.hpp"

class ArcadeCabinet {
    IntCodeVM m_computer;

    enum class Object { Empty, Wall, Block, Paddle, Ball };

    std::map<std::pair<int, int>, Object> m_state;

public:
    ArcadeCabinet(const char* filepath) : m_computer(filepath) { m_computer.write_memory(0, 2); }

    tick(int input)
    {
        auto output = m_computer.continue_execution(input);

        while (output) {
        }
    }
};

int main(void)
{
    ArcadeCabinet arcade("../inputs/13.txt");
    arcade.tick(0);

    int user_input_code;
    while (true) {
        std::cin >> input_code;

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
