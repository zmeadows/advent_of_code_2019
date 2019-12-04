#include <assert.h>

#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <vector>

struct Segment {
    int x1;
    int y1;
    int x2;
    int y2;
    int total_wire_length;
};

std::ostream& operator<<(std::ostream& os, const Segment& s)
{
    os << "(" << s.x1 << "," << s.y1 << ") -> (" << s.x2 << "," << s.y2 << ")";
    return os;
}

std::optional<std::pair<int, int>> find_intersection_manhattan_distance_from_origin(Segment s1,
                                                                                    Segment s2)
{
    char o1 = '!';
    char o2 = '!';

    // ensure there are no overlapping vertical wires
    if (s1.x1 == s1.x2 && s2.x1 == s2.x2 && s2.x1 == s1.x1) {
        assert(s1.y1 > s2.y2 || s1.y2 < s2.y1);
    }

    // ensure there are no overlapping horizontal wires
    if (s1.y1 == s1.y2 && s2.y1 == s2.y2 && s2.y1 == s1.y1) {
        assert(s1.x1 > s2.x2 || s1.x2 < s2.x1);
    }

    if (s1.x1 == s1.x2) {
        assert(s1.y1 != s1.y2);
        o1 = 'v';
    }

    if (s1.y1 == s1.y2) {
        assert(s1.x1 != s1.x2);
        o1 = 'h';
    }

    if (s2.x1 == s2.x2) {
        assert(s2.y1 != s2.y2);
        o2 = 'v';
    }

    if (s2.y1 == s2.y2) {
        assert(s2.x1 != s2.x2);
        o2 = 'h';
    }

    assert(o1 != '!' && o2 != '!');

    if (o1 == o2) {
        return {};
    }

    if (o1 == 'v' && o2 == 'h') {
        std::swap(s1, s2);
    }

    // now, o2 == v, o1 == h

    assert(s2.x1 == s2.x2);
    assert(s1.y1 == s1.y2);

    const int vx = s2.x1;
    const int hy = s1.y1;

    const auto os1 = s1;
    const auto os2 = s2;

    if (s1.x1 > s1.x2) std::swap(s1.x1, s1.x2);
    if (s2.y1 > s2.y2) std::swap(s2.y1, s2.y2);

    if (s1.x1 <= vx && s1.x2 >= vx && s2.y1 <= hy && s2.y2 >= hy) {
        const int ix = s2.x1;
        const int iy = s1.y1;
        const int leftover_x = std::abs(os1.x2 - ix);
        const int leftover_y = std::abs(os2.y2 - iy);

        return std::pair<int, int>(
            std::abs(ix) + std::abs(iy),
            s1.total_wire_length + s2.total_wire_length - leftover_x - leftover_y);
    }

    return {};
}

using WirePath = std::vector<Segment>;

WirePath parse_wire_path(const std::string& encoded_path)
{
    std::istringstream ss(encoded_path);
    std::string segment_token;

    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    int total_wire_length = 0;
    WirePath wire_path;
    wire_path.reserve(encoded_path.size() / 5);

    auto move_wire_tip = [&](char dir, int len) {
        x1 = x2;
        y1 = y2;
        total_wire_length += len;

        switch (dir) {
            case 'L':
                x2 -= len;
                break;
            case 'R':
                x2 += len;
                break;
            case 'U':
                y2 += len;
                break;
            case 'D':
                y2 -= len;
                break;
        };

        wire_path.push_back({x1, y1, x2, y2, total_wire_length});
    };

    while (std::getline(ss, segment_token, ',')) {
        const char dir = segment_token[0];
        const int len = std::stoi(segment_token.substr(1, segment_token.size() - 1));
        move_wire_tip(dir, len);
    }

    return wire_path;
}

int main(void)
{
    std::ifstream infile("../inputs/3.txt");

    std::string first_wire_path_encoding;
    std::string second_wire_path_encoding;

    std::getline(infile, first_wire_path_encoding, '\n');
    std::getline(infile, second_wire_path_encoding, '\n');

    WirePath first_wire_path = parse_wire_path(first_wire_path_encoding);
    WirePath second_wire_path = parse_wire_path(second_wire_path_encoding);

    int min_crossing_distance = std::numeric_limits<int>::max();
    int min_wire_travel = std::numeric_limits<int>::max();

    for (const Segment& s1 : first_wire_path) {
        for (const Segment& s2 : second_wire_path) {
            const auto d = find_intersection_manhattan_distance_from_origin(s1, s2);
            if (d) {
                std::cout << "Segment1: " << s1 << std::endl;
                std::cout << "Segment2: " << s2 << std::endl;
                std::cout << "Distance: " << d->first << std::endl;
                std::cout << "Total Wire Travel: " << d->second << std::endl;
                std::cout << std::endl;

                if (d->first < min_crossing_distance) {
                    min_crossing_distance = d->first;
                }
                if (d->second < min_wire_travel) {
                    min_wire_travel = d->second;
                }
            }
        }
    }

    std::cout << "minimum crossing manhattan distance: " << min_crossing_distance << std::endl;
    std::cout << "minimum wire travel: " << min_wire_travel << std::endl;

    return 0;
}
