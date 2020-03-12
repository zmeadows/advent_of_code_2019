#include "prelude.hpp"

// {{{ CHEMICAL
class Chemical {
public:
    using Rep = uint64_t;

private:
    static inline Rep hash_resource_label(std::string_view label)
    {
        const Rep h = std::hash<std::string_view>{}(label);
        auto it = s_unhash.find(h);
        if (it == s_unhash.end()) {
            s_unhash[h] = std::string(label);
        }
        return h;
    }

    Rep m_id;

    static ska::flat_hash_map<Rep, std::string> s_unhash;

public:
    Chemical(void) = delete;
    Chemical(std::string_view label) : m_id(hash_resource_label(label)) {}

    friend inline bool operator==(const Chemical& r1, const Chemical& r2) { return r1.m_id == r2.m_id; }
    friend inline bool operator!=(const Chemical& r1, const Chemical& r2) { return !(r1 == r2); }

    inline Rep id(void) const { return m_id; }

    inline std::string_view name(void) const
    {
        auto it = s_unhash.find(m_id);
        assert(it != s_unhash.end());
        return it->second;
    }
};

ska::flat_hash_map<Chemical::Rep, std::string> Chemical::s_unhash = {};

namespace std {
template <>
struct hash<Chemical> {
    size_t operator()(const Chemical& res) const { return std::hash<Chemical::Rep>{}(res.id()); };
};
}  // namespace std

// }}}

struct ReactionElement {
    Chemical resource;
    uint64_t count;

    ReactionElement(void) = delete;
    ReactionElement(std::string_view label, uint64_t count_) : resource(label), count(count_) {}
};

struct Reaction {
    std::vector<ReactionElement> inputs;
    uint64_t yield;
};

using ReactionSet = ska::flat_hash_map<Chemical, Reaction>;

class Surplus {
    ska::flat_hash_map<Chemical, uint64_t> m_counts;

public:
    void assign(Chemical c, uint64_t count)
    {
        auto it = m_counts.find(c);
        if (it != m_counts.end()) {
            it->second++;
        }
        else {
            m_counts[c] = 1;
        }
    }

    uint64_t count(Chemical c) const
    {
        auto it = m_counts.find(c);
        if (it == m_counts.end()) {
            return 0;
        }
        else {
            return it->second;
        }
    }
};

uint64_t compute_ore_cost(const ReactionSet& reactions, const Surplus& surplus, Chemical product,
                          uint64_t quantity)
{
    Surplus new_surplus;

    const Reaction& production_reaction = reactions.at(product);
    const uint64_t product_stock = surplus.count(product);
    const uint64_t production_requirement = quantity > product_stock ? quantity - product_stock : 0;

    const uint64_t reaction_runs_required =
        production_requirement == 0
            ? 0
            : (production_requirement + production_reaction.yield - 1) / production_reaction.yield;

    const uint64_t new_surplus =
        reaction_runs_required * production_reaction.yield - production_requirement;

    new_surplus.assign(product, new_surplus);

    uint64_t ore_required = 0;

    for (const auto& [iresource, icount] : production_reaction.inputs) {
        const uint64_t input_requirement = reaction_runs_required * icount;
        if (iresource == Chemical("ORE")) {
            ore_required += input_requirement;
        }
        else {
            ore_required += compute_ore_cost(reactions, surplus, iresource, input_requirement);
        }
    }

    return ore_required;
}

void solve_part_one(const ReactionSet& reactions)
{
    Surplus surplus;
    const auto answer = compute_ore_cost(reactions, surplus, Chemical("FUEL"), 1);
    std::cout << "part one answer =  " << answer << std::endl;
}

int main(void)
{
    ska::flat_hash_map<Chemical, Reaction> reactions;

    std::cout << "Reading input reactions..." << std::endl;
    for_each_line_in_file("../inputs/14.txt", [&](std::string_view line) {
        std::cout << line << std::endl;
        const auto [in_str, out_elem_str] = split(line, "=>");
        const auto in_elem_strs = split_on(in_str, ",");

        auto convert_elem_str = [](std::string_view elem_str) -> ReactionElement {
            const auto [count_str, resource_str] = split(elem_str, " ");
            assert(!count_str.empty() || !elem_str.empty());
            return ReactionElement(resource_str, *convert_string<uint64_t>(count_str));
        };

        ReactionElement output = convert_elem_str(out_elem_str);

        // read the reaction inputs
        std::vector<ReactionElement> inputs;
        inputs.reserve(in_elem_strs.size());
        for (auto elem_str : in_elem_strs) {
            inputs.emplace_back(convert_elem_str(elem_str));
        }

        reactions.emplace(output.resource, Reaction{std::move(inputs), output.count});
    });
    std::cout << "... finished reading input reactions." << std::endl;

    solve_part_one(reactions);

    return 0;
}
