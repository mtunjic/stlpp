#pragma once

#include "concepts.h"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace mt {

/// Assign `value` to every element in `[first, last)` (EoP ch. 9).
template <std::forward_iterator I, typename T>
    requires std::indirectly_writable<I, const T&>
[[nodiscard]] constexpr I fill(I first, I last, const T& value)
{
    std::fill(first, last, value);
    return last;
}

/// Assign `value` to `n` elements starting at `first`.
template <std::forward_iterator I, Integer N, typename T>
    requires std::indirectly_writable<I, const T&>
[[nodiscard]] constexpr I fill_n(I first, N n, const T& value)
{
    return std::fill_n(first, n, value);
}

template <std::ranges::forward_range R, typename T>
void fill(R& range, const T& value)
{
    (void)fill(std::ranges::begin(range), std::ranges::end(range), value);
}

}  // namespace mt
