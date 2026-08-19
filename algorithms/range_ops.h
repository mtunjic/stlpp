#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Consecutive overlapping pairs: (a0,a1), (a1,a2), ...
///
/// Flow: walk two iterators one step apart; each step writes one pair.
/// Iterator strength: forward. Complexity: O(n). Empty or one element: no pairs.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   auto p = mt::pairwise(v);  // {(1,2), (2,3), (3,4)}
template <std::forward_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O>
    requires std::indirectly_writable<O, std::pair<Value_type<I>, Value_type<I>>>
O pairwise(I first, S last, O dest)
{
    if (first == last) {
        return dest;
    }
    I next = first;
    ++next;
    for (; next != last; ++first, ++next) {
        *dest = std::pair<Value_type<I>, Value_type<I>>{*first, *next};
        ++dest;
    }
    return dest;
}

template <std::ranges::forward_range R>
[[nodiscard]] auto pairwise(R&& range)
{
    using T = std::ranges::range_value_t<R>;
    std::vector<std::pair<T, T>> out;
    pairwise(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out));
    return out;
}

/// Copy `[a, a_last)` then `[b, b_last)` into `dest`.
///
/// Example:
///   std::vector<int> a{1, 2}, b{3};
///   auto c = mt::concat(a, b);  // {1, 2, 3}
template <std::input_iterator I1, std::sentinel_for<I1> S1,
          std::input_iterator I2, std::sentinel_for<I2> S2, std::weakly_incrementable O>
    requires std::indirectly_writable<O, Value_type<I1>> &&
             std::indirectly_writable<O, Value_type<I2>>
O concat(I1 a, S1 a_last, I2 b, S2 b_last, O dest)
{
    for (; a != a_last; ++a, ++dest) {
        *dest = *a;
    }
    for (; b != b_last; ++b, ++dest) {
        *dest = *b;
    }
    return dest;
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
    requires std::same_as<std::ranges::range_value_t<R1>, std::ranges::range_value_t<R2>>
[[nodiscard]] auto concat(R1&& a, R2&& b)
{
    std::vector<std::ranges::range_value_t<R1>> out;
    concat(std::ranges::begin(a), std::ranges::end(a),
           std::ranges::begin(b), std::ranges::end(b), std::back_inserter(out));
    return out;
}

/// Write elements of `[first, last)` with `sep` between them (Haskell intercalate).
///
/// Example:
///   std::vector<char> letters{'a', 'b', 'c'};
///   std::string s;
///   mt::intercalate(',', letters.begin(), letters.end(), std::back_inserter(s));
///   // s == "a,b,c"
template <typename T, std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O>
    requires std::indirectly_writable<O, T> && std::indirectly_writable<O, Value_type<I>>
O intercalate(const T& sep, I first, S last, O dest)
{
    if (first == last) {
        return dest;
    }
    *dest = *first;
    ++dest;
    ++first;
    for (; first != last; ++first) {
        *dest = sep;
        ++dest;
        *dest = *first;
        ++dest;
    }
    return dest;
}

template <typename T, std::ranges::input_range R>
[[nodiscard]] auto intercalate(const T& sep, R&& range)
{
    std::vector<T> out;
    intercalate(sep, std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out));
    return out;
}

/// Consecutive equal runs (Haskell group). Each group is a vector of copies.
///
/// Flow: while input remains, copy the current value until a different one
/// appears; emit that run and continue.
///
/// Example:
///   mt::group(std::vector<int>{1, 1, 2, 2, 2, 1});
///   // {{1,1}, {2,2,2}, {1}}
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O>
O group(I first, S last, O dest)
{
    while (first != last) {
        std::vector<Value_type<I>> run;
        auto value = *first;
        do {
            run.push_back(value);
            ++first;
        } while (first != last && *first == value);
        *dest = std::move(run);
        ++dest;
    }
    return dest;
}

template <std::ranges::input_range R>
[[nodiscard]] auto group(R&& range)
{
    std::vector<std::vector<std::ranges::range_value_t<R>>> out;
    group(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out));
    return out;
}

/// Left-rotate by `n` (negative `n` rotates right). `n` is reduced modulo length.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4, 5};
///   mt::shift_by(v, 2);   // {3, 4, 5, 1, 2}
///   mt::shift_by(v, -1);  // {2, 3, 4, 5, 1}
template <std::forward_iterator I>
void shift_by(I first, I last, Difference_type<I> n)
{
    const auto len = std::distance(first, last);
    if (len == 0) {
        return;
    }
    n %= len;
    if (n < 0) {
        n += len;
    }
    if (n == 0) {
        return;
    }
    std::rotate(first, std::next(first, n), last);
}

template <std::ranges::forward_range R>
void shift_by(R&& range, std::ranges::range_difference_t<R> n)
{
    shift_by(std::ranges::begin(range), std::ranges::end(range), n);
}

}  // namespace mt
