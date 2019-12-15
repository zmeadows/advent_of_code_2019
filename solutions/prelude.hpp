#include <assert.h>

#include <charconv>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include "flat_hash_map/flat_hash_map.hpp"

using i64 = int64_t;
using i32 = int32_t;
using u64 = uint64_t;
using u32 = uint32_t;

template <typename Callable>
void for_each_line_in_file(const char* filepath, Callable&& f)
{
    std::ifstream infile(filepath);
    std::string line_buffer;
    while (std::getline(infile, line_buffer)) {
        f(line_buffer);
    }
}

namespace {

const std::string WHITESPACE = " \n\r\t\f\v";

std::string_view ltrim(std::string_view s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string_view::npos) ? "" : s.substr(start);
}

std::string_view rtrim(std::string_view s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string_view::npos) ? "" : s.substr(0, end + 1);
}

std::string_view trim(std::string_view s) { return rtrim(ltrim(s)); }

}  // namespace

std::vector<std::string_view> split_on(std::string_view s, std::string_view delim = " ")
{
    std::vector<std::string_view> result;

    while (true) {
        const size_t n = s.find(delim);
        if (n == std::string_view::npos) {
            result.push_back(trim(s.substr(0, s.size())));
            return result;
        }
        else {
            result.push_back(trim(s.substr(0, n)));
            s = s.substr(n + delim.size(), s.size());
        }
    }
}

std::pair<std::string_view, std::string_view> split(std::string_view s, std::string_view delim = " ")
{
    const size_t n = s.find(delim);
    if (n == std::string_view::npos) {
        return {s, ""};
    }
    else {
        return {trim(s.substr(0, n)), trim(s.substr(n + delim.size(), s.size()))};
    }
}

template <typename T>
std::optional<T> convert_string(std::string_view str)
{
    T value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
        return {};
    }
    return value;
}
