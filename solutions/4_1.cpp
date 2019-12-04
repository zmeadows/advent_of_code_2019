#include <algorithm>
#include <iostream>
#include <vector>

std::vector<int> digits_of(int n)
{
    std::vector<int> digits;
    while (n > 0) {
        digits.push_back(n % 10);

        n /= 10;
    }
    std::reverse(digits.begin(), digits.end());
    return digits;
}

bool matches_password_criteria(int n)
{
    const auto digits = digits_of(n);

    if (digits.size() != 6) {
        return false;
    }

    int last_digit = -1;
    bool has_adjacent_double_digits = false;
    for (auto d : digits) {
        if (d == last_digit) {
            has_adjacent_double_digits = true;
        }

        if (d < last_digit) {
            return false;
        }
        last_digit = d;
    }

    if (!has_adjacent_double_digits) {
        return false;
    }

    return true;
}

int main(void)
{
    int count = 0;
    for (int n = 138241; n <= 674034; n++) {
        if (matches_password_criteria(n)) {
            count += 1;
        }
    }

    std::cout << count << std::endl;
    return 0;
}
