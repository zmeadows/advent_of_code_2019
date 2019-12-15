#include <fstream>
#include <functional>
#include <iostream>

class Chemical {
public:
    using ID = uint64_t;

private:
    const ID m_id;

public:
    Chemical(void) = delete;
    Chemical(const std::string& label) : m_id(std::hash<std::string>{}()) {}
};

struct ReactionElement {
    size_t count;
    Chemical chemical;

    ReactionElement(void) = delete;
    ReactionElement(size_t _count, const std::string& label) : count(_count), chemical(label) {}
};

class NanoFactorySim {
    const std::map<ReactionElement, std::vector<ReactionElement>> m_reactions;

    static std::map<ReactionElement, std::vector<ReactionElement>> read_reaction_file(
        const char* reaction_file)
    {
        std::ifstream(reaction_file);
    }

public:
    NanoFactorySim(const char* reaction_file) : m_reactions(read_reaction_file(reaction_file)) {}

    void run(void);
};

int main(void)
{
    NanoFactory("../inputs/14.txt");

    return 0;
}
