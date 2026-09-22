#pragma once

#include "concepts.h"

#include <iterator>
#include <utility>

namespace mt {

/// Advance `i` by `n` steps. Uses `+=` for random-access, `++`/`--` otherwise.
///
/// Example:
///   auto it = v.begin();
///   mt::advance(it, 2);  // *it == v[2]
template <std::input_iterator I>
constexpr void advance(I& i, Difference_type<I> n)
{
    if constexpr (std::random_access_iterator<I>) {
        i += n;
    } else if constexpr (std::bidirectional_iterator<I>) {
        if (n > 0) {
            while (n--) {
                ++i;
            }
        } else {
            while (n++) {
                --i;
            }
        }
    } else {
        while (n > 0) {
            ++i;
            --n;
        }
    }
}

/// Advance `i` by at most `n` steps, stopping at `last`. Negative `n` uses
/// the unbounded overload (input iterators ignore it).
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr void advance(I& i, S last, Difference_type<I> n)
{
    if (n <= 0) {
        mt::advance(i, n);
        return;
    }
    if constexpr (std::random_access_iterator<I> && std::sized_sentinel_for<S, I>) {
        const auto d = last - i;
        if (d <= 0) {
            return;
        }
        i += n < d ? n : d;
    } else {
        while (n > 0 && i != last) {
            ++i;
            --n;
        }
    }
}

/// How many of the next `n` elements of `[first, last)` actually exist.
template <std::forward_iterator I, std::sentinel_for<I> S, Integer N>
[[nodiscard]] constexpr N bounded_count(I first, S last, N n)
{
    if (n <= N{0}) {
        return N{0};
    }
    if constexpr (std::sized_sentinel_for<S, I>) {
        const auto d = last - first;
        if (d <= 0) {
            return N{0};
        }
        return std::cmp_less(+n, d) ? n : static_cast<N>(d);
    } else {
        N k = 0;
        while (k < n && first != last) {
            ++first;
            ++k;
        }
        return k;
    }
}

/// Copy of `i` moved one step forward (EoP successor).
///
/// Example:
///   auto n = mt::successor(v.begin());  // &v[1]
template <std::input_iterator I>
[[nodiscard]] constexpr I successor(I i)
{
    ++i;
    return i;
}

/// Copy of `i` moved `n` steps (may be negative if bidirectional).
///
/// Example:
///   auto mid = mt::successor(v.begin(), v.size() / 2);
template <std::input_iterator I>
[[nodiscard]] constexpr I successor(I i, Difference_type<I> n)
{
    mt::advance(i, n);
    return i;
}

/// Copy of `i` moved one step backward.
///
/// Example:
///   auto last = mt::predecessor(v.end());  // &v.back()
template <std::bidirectional_iterator I>
[[nodiscard]] constexpr I predecessor(I i)
{
    --i;
    return i;
}

}  // namespace mt
