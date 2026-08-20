#pragma once

#include "concepts.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Iterator pair covering `count` elements starting at offset `start`.
/// `start` and `count` are clipped to the range (no wrap-around).
///
/// Example:
///   std::vector<int> v{0, 1, 2, 3, 4};
///   auto [a, b] = mt::slice(v.begin(), v.end(), 1, 3);
///   // [a, b) == {1, 2, 3}
template <std::random_access_iterator I>
[[nodiscard]] constexpr std::pair<I, I> slice(I first, I last, Difference_type<I> start,
                                              Difference_type<I> count)
{
    const Difference_type<I> n = last - first;
    if (start < 0) {
        start = 0;
    }
    if (start > n) {
        start = n;
    }
    if (count < 0) {
        count = 0;
    }
    const Difference_type<I> remaining = n - start;
    if (count > remaining) {
        count = remaining;
    }
    return {first + start, first + start + count};
}

template <std::ranges::random_access_range R>
[[nodiscard]] auto slice(R& range, std::ranges::range_difference_t<R> start,
                         std::ranges::range_difference_t<R> count)
{
    auto [a, b] = slice(std::ranges::begin(range), std::ranges::end(range), start, count);
    return std::ranges::subrange(a, b);
}

}  // namespace mt
