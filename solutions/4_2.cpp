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

    int current_match_digit = -1;
    int current_match_length = 0;
    bool found_double = false;

    for (auto d : digits) {
        if (d < current_match_digit) {
            return false;
        }

        if (d == current_match_digit) {
            current_match_length++;
        }
        else {
            if (current_match_length == 2) {
                found_double = true;
            }
            current_match_length = 1;
            current_match_digit = d;
        }
    }

    // in case the matching digits were the last pair in the number
    if (current_match_length == 2) {
        found_double = true;
    }

    if (!found_double) {
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
