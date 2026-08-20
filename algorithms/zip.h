#pragma once

#include "concepts.h"

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Write `n` pairs `(*i, *j)` to `dest`. Stops after `n` steps.
///
/// Example:
///   std::vector<int> a{1, 2}, b{10, 20};
///   std::vector<std::pair<int, int>> out;
///   mt::zip_n(a.begin(), b.begin(), 2, std::back_inserter(out));
template <std::input_iterator I1, std::input_iterator I2, Integer N, std::weakly_incrementable O>
    requires std::indirectly_writable<O, std::pair<Value_type<I1>, Value_type<I2>>>
[[nodiscard]] constexpr O zip_n(I1 first1, I2 first2, N n, O dest)
{
    while (n > N{0}) {
        *dest = std::pair<Value_type<I1>, Value_type<I2>>{*first1, *first2};
        ++first1;
        ++first2;
        ++dest;
        --n;
    }
    return dest;
}

/// Write at most `n` pairs from `[first1, last1)` and `[first2, last2)`.
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, Integer N, std::weakly_incrementable O>
    requires std::indirectly_writable<O, std::pair<Value_type<I1>, Value_type<I2>>>
[[nodiscard]] constexpr O zip_n(I1 first1, S1 last1, I2 first2, S2 last2, N n, O dest)
{
    while (n > N{0} && first1 != last1 && first2 != last2) {
        *dest = std::pair<Value_type<I1>, Value_type<I2>>{*first1, *first2};
        ++first1;
        ++first2;
        ++dest;
        --n;
    }
    return dest;
}

/// Zip two ranges until either ends.
///
/// Example:
///   auto z = mt::zip(a, b);  // vector of pairs
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O>
O zip(I1 first1, S1 last1, I2 first2, S2 last2, O dest)
{
    while (first1 != last1 && first2 != last2) {
        *dest = std::pair<Value_type<I1>, Value_type<I2>>{*first1, *first2};
        ++first1;
        ++first2;
        ++dest;
    }
    return dest;
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
[[nodiscard]] auto zip(R1&& a, R2&& b)
{
    using T1 = std::ranges::range_value_t<R1>;
    using T2 = std::ranges::range_value_t<R2>;
    std::vector<std::pair<T1, T2>> out;
    zip(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b), std::ranges::end(b),
        std::back_inserter(out));
    return out;
}

/// Like zip, but write `op(*i, *j)` instead of a pair.
///
/// Example:
///   std::vector<int> a{1, 2}, b{10, 20}, out;
///   mt::zip_with(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(out),
///                std::plus<int>{});
///   // out == {11, 22}
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Op>
O zip_with(I1 first1, S1 last1, I2 first2, S2 last2, O dest, Op op)
{
    while (first1 != last1 && first2 != last2) {
        *dest = std::invoke(op, *first1, *first2);
        ++first1;
        ++first2;
        ++dest;
    }
    return dest;
}

/// Split pairs into two output ranges.
///
/// Example:
///   std::vector<std::pair<int, int>> z{{1, 10}, {2, 20}};
///   std::vector<int> a(2), b(2);
///   mt::unzip(z.begin(), z.end(), a.begin(), b.begin());
///   // a == {1, 2}, b == {10, 20}
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O1,
          std::weakly_incrementable O2>
[[nodiscard]] constexpr std::pair<O1, O2> unzip(I first, S last, O1 dest1, O2 dest2)
{
    for (; first != last; ++first) {
        *dest1 = (*first).first;
        *dest2 = (*first).second;
        ++dest1;
        ++dest2;
    }
    return {dest1, dest2};
}

}  // namespace mt
