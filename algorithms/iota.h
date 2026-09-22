#pragma once

#include "concepts.h"
#include "iterator_ops.h"

#include <iterator>
#include <ranges>

namespace mt {

/// Write `value, value+1, ...` into `[first, last)`.
///
/// Example:
///   std::vector<int> v(4);
///   mt::iota(v.begin(), v.end(), 10);  // {10, 11, 12, 13}
template <std::forward_iterator I, std::sentinel_for<I> S>
    requires std::incrementable<Value_type<I>>
constexpr void iota(I first, S last, Value_type<I> value = {})
{
    while (first != last) {
        *first = value;
        if (++first != last) {
            ++value;
        }
    }
}

template <std::ranges::forward_range R>
    requires std::incrementable<std::ranges::range_value_t<R>>
void iota(R&& range, std::ranges::range_value_t<R> value = {})
{
    mt::iota(std::ranges::begin(range), std::ranges::end(range), std::move(value));
}

/// Decreasing fill: n-1, n-2, ..., 0 for a range of length n.
///
/// Example:
///   std::vector<int> v(4);
///   mt::reverse_iota(v.begin(), v.end());  // {3, 2, 1, 0}
template <std::forward_iterator I>
    requires std::incrementable<Value_type<I>>
void reverse_iota(I first, I last)
{
    auto n = std::distance(first, last);
    auto value = static_cast<Value_type<I>>(n);
    for (; first != last; ++first) {
        --value;
        *first = value;
    }
}

/// Rise then fall: `0,1,...,k-1, k-1,...,0` (needs even or odd n).
///
/// Example:
///   std::vector<int> v(5);
///   mt::hill(v.begin(), v.end());
template <std::forward_iterator I>
void hill(I first, I last)
{
    const auto n = std::distance(first, last);
    const I middle = mt::successor(first, n / 2);
    mt::iota(first, middle);
    mt::reverse_iota(middle, last);
}

/// Fall then rise.
///
/// Example:
///   std::vector<int> v(5);
///   mt::valley(v.begin(), v.end());  // {1, 0, 0, 1, 2}
template <std::forward_iterator I>
void valley(I first, I last)
{
    const auto n = std::distance(first, last);
    const I middle = mt::successor(first, n / 2);
    mt::reverse_iota(first, middle);
    mt::iota(middle, last);
}

}  // namespace mt
