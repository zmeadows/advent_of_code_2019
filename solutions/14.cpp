#include "prelude.hpp"

class Resource {
public:
    using TypeRep = uint64_t;

private:
    static inline TypeRep hash_resource_label(std::string_view label)
    {
        const TypeRep h = std::hash<std::string_view>{}(label);
        auto it = s_unhash.find(h);
        if (it == s_unhash.end()) {
            s_unhash[h] = std::string(label);
        }
        return h;
    }

    TypeRep m_id;

    static ska::flat_hash_map<TypeRep, std::string> s_unhash;

public:
    Resource(void) = delete;
    Resource(std::string_view label) : m_id(hash_resource_label(label)) {}

    friend inline bool operator==(const Resource& r1, const Resource& r2) { return r1.m_id == r2.m_id; }
    friend inline bool operator!=(const Resource& r1, const Resource& r2) { return !(r1 == r2); }

    inline TypeRep id(void) const { return m_id; }

    inline std::string_view name(void) const
    {
        auto it = s_unhash.find(m_id);
        assert(it != s_unhash.end());
        return it->second;
    }
};

ska::flat_hash_map<Resource::TypeRep, std::string> Resource::s_unhash = {};

namespace std {
template <>
struct hash<Resource> {
    size_t operator()(const Resource& res) const { return std::hash<Resource::TypeRep>{}(res.id()); };
};
}  // namespace std

struct ReactionElement {
    Resource resource;
    uint64_t count;

    ReactionElement(void) = delete;
    ReactionElement(std::string_view label, uint64_t count_) : resource(label), count(count_) {}
};

struct Reaction {
    std::vector<ReactionElement> inputs;
    uint64_t yield;
};

using ReactionSet = ska::flat_hash_map<Resource, Reaction>;

class Inventory {
    ska::flat_hash_map<Resource, uint64_t> m_counts;

public:
    void add_resource(Resource resource, uint64_t count)
    {
        auto it = m_counts.find(resource);
        if (it != m_counts.end()) {
            it->second++;
        }
        else {
            m_counts[resource] = 1;
        }
    }

    void remove_resource(Resource resource, uint64_t count_to_remove)
    {
        auto it = m_counts.find(resource);
        assert(it != m_counts.end());

        uint64_t& current_count = it->second;
        assert(count_to_remove <= current_count);

        if (current_count == count_to_remove) {
            m_counts.erase(resource);
        }
        else {
            current_count -= count_to_remove;
        }
    }

    void set_resource(Resource resource, uint64_t count) { m_counts[resource] = count; }

    uint64_t available(Resource resource) const
    {
        auto it = m_counts.find(resource);
        if (it == m_counts.end()) {
            return 0;
        }
        else {
            return it->second;
        }
    }
};

void follow_reaction_chain(const ReactionSet& reactions, Inventory& inventory,
                           Resource production_resource, uint64_t reaction_requirement)
{
    const Reaction& production_reaction = reactions.at(goal_resource);
    if (goal_reaction.inputs.size()

    const uint64_t production_requirement = goal_yield - inventory.available(goal_resource);

    // ceiling division (we have to over-produce some inputs)
    const uint64_t reaction_runs_required =
        (production_requirement + goal_reaction.yield - 1) / goal_reaction.yield;
    const uint64_t output_yield = reactions_runs_required * goal_reaction.yield;
    assert(output_yield > production_requirement);

    for (const auto& [iresource, icount] : goal_reaction.inputs) {
        const uint64_t iyield = icount * reactions_required;
        if (iresource == Resource("ORE")) {
        }
        follow_reaction_chain(reactions, input_resource, input_count_required, ore_count);
    }

    inventory.set_resource(goal_resource, output_yield - goal_yield);
}

void solve_part_one(const ReactionSet& reactions)
{
    Inventory inventory;
    follow_reaction_chain(reactions, post_ore_inventory, Resource("FUEL"), 1, ore_count);
    std::cout << "part one answer =  " << ore_count << std::endl;
}

int main(void)
{
    ska::flat_hash_map<Resource, Reaction> reactions;

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
