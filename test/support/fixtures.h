#pragma once

#include "test_concepts.h"

#include <numeric>
#include <vector>

namespace mt::test {

inline std::vector<int> iota_vector(int n, int start = 0)
{
    std::vector<int> v(static_cast<std::size_t>(n));
    std::iota(v.begin(), v.end(), start);
    return v;
}

struct CountedCases {
    int n_within;
    int n_over;
    int n_negative;
};

inline CountedCases default_counted_cases(int size)
{
    return CountedCases{size / 2, size + 10, -1};
}

template <typename Container>
std::vector<int> to_vector(const Container& c)
{
    return {c.begin(), c.end()};
}

template <typename Container>
std::vector<int> to_filled_vector(Container& c, int fill)
{
    return {c.begin(), filled_end(c, fill)};
}

template <typename Range, typename T, typename Op>
T std_accumulate_oracle(const Range& range, T init, Op op)
{
    T acc = init;
    for (const auto& x : range) {
        acc = op(std::move(acc), x);
    }
    return acc;
}

template <typename Range>
std::vector<typename Range::value_type> std_partial_sum_oracle(const Range& range)
{
    std::vector<typename Range::value_type> out;
    std::partial_sum(range.begin(), range.end(), std::back_inserter(out));
    return out;
}

}  // namespace mt::test
