#include <assert.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

struct OrbitTree {
    size_t m_id;
    std::vector<std::unique_ptr<OrbitTree>> m_children;
    OrbitTree* m_parent;

    static std::map<size_t, std::string> names;

    OrbitTree(size_t _id, OrbitTree* _parent) : m_id(_id), m_parent(_parent) {}

    void count_orbits_helper(size_t current_depth, size_t& indirect_count, size_t& direct_count)
    {
        direct_count += m_children.size();

        if (current_depth > 1) {
            indirect_count += current_depth - 1;
        }

        for (auto& child : m_children) {
            child->count_orbits_helper(current_depth + 1, indirect_count, direct_count);
        }
    };

    std::optional<size_t> distance_to(size_t end_id, std::optional<size_t> ignore_child_node,
                                      size_t distance_so_far, bool try_parent)
    {
        if (m_id == end_id) {
            return distance_so_far;
        }

        for (auto& child : m_children) {
            if (ignore_child_node && child->m_id == *ignore_child_node) {
                continue;
            }
            auto d = child->distance_to(end_id, {}, distance_so_far + 1, false);
            if (d) {
                return d;
            }
        }

        if (try_parent && m_parent) {
            return m_parent->distance_to(end_id, m_id, distance_so_far + 1, true);
        }
        else {
            return {};
        }
    }

    OrbitTree* get_com_node(void)
    {
        OrbitTree* node = this;
        while (node->m_parent) {
            node = node->m_parent;
        };
        return node;
    };

    OrbitTree* find_node_helper(size_t find_id)
    {
        if (m_id == find_id) {
            return this;
        }
        else {
            for (auto& child : m_children) {
                OrbitTree* ptr = child->find_node_helper(find_id);
                if (ptr) return ptr;
            }
        }
        return nullptr;
    }

    OrbitTree* find_node(size_t find_id) { return get_com_node()->find_node_helper(find_id); }

    bool insert_helper(size_t parent_id, size_t child_id)
    {
        if (m_id == parent_id) {
            m_children.emplace_back(std::make_unique<OrbitTree>(child_id, this));
            return true;
        }
        else {
            for (auto& child : m_children) {
                if (child->insert_helper(parent_id, child_id)) {
                    return true;
                }
            }
        }

        return false;
    }

public:
    OrbitTree(size_t com_id) : m_id(com_id), m_parent(nullptr) {}

    void insert(size_t parent_id, size_t child_id)
    {
        assert(parent_id != child_id);
        bool was_inserted = get_com_node()->insert_helper(parent_id, child_id);
        assert(was_inserted);
    }

    std::pair<size_t, size_t> count_orbits(void)
    {
        size_t indirect = 0;
        size_t direct = 0;
        get_com_node()->count_orbits_helper(0, indirect, direct);

        return {direct, indirect};
    };

    std::optional<size_t> distance_between(size_t start_id, size_t end_id)
    {
        OrbitTree* start_node = this->find_node(start_id);
        if (start_node == nullptr) return {};

        return start_node->distance_to(end_id, {}, 0, true);
    }
};

std::map<size_t, std::string> OrbitTree::names = std::map<size_t, std::string>();

int main(void)
{
    std::ifstream infile("../inputs/6.txt");
    std::string orbit_str;

    std::multimap<size_t, size_t> orbit_map;
    std::map<size_t, std::string> planet_hash_name_map;
    size_t com_hash, you_orbiting_hash, santa_orbiting_hash;

    while (std::getline(infile, orbit_str)) {
        const size_t paren_pos = orbit_str.find(")");
        const std::string o1 = orbit_str.substr(0, paren_pos);
        const std::string o2 = orbit_str.substr(paren_pos + 1, orbit_str.size());

        const size_t o1_hash = std::hash<std::string>{}(o1);
        const size_t o2_hash = std::hash<std::string>{}(o2);

        OrbitTree::names[o1_hash] = o1;
        OrbitTree::names[o2_hash] = o2;

        if (o1 == "COM") com_hash = o1_hash;
        if (o2 == "YOU") you_orbiting_hash = o1_hash;
        if (o2 == "SAN") santa_orbiting_hash = o1_hash;

        orbit_map.emplace(o1_hash, o2_hash);
    }

    OrbitTree tree(com_hash);

    std::function<void(size_t)> fill_tree;

    fill_tree = [&](size_t parent_id) {
        auto child_itr = orbit_map.equal_range(parent_id);
        for (auto it = child_itr.first; it != child_itr.second; it++) {
            assert(parent_id == it->first);
            tree.insert(parent_id, it->second);
            fill_tree(it->second);
        }
    };

    fill_tree(com_hash);

    auto [direct_orbits, indirect_orbits] = tree.count_orbits();

    assert(direct_orbits == orbit_map.size());

    std::cout << "direct: " << direct_orbits << std::endl;
    std::cout << "indirect: " << indirect_orbits << std::endl;
    std::cout << "total: " << direct_orbits + indirect_orbits << std::endl;

    auto d = tree.distance_between(you_orbiting_hash, santa_orbiting_hash);
    assert(d);
    std::cout << "you -> santa distance: " << *d << std::endl;
}
