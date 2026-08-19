#pragma once

#include "concepts.h"

#include <iterator>

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
        while (n--) {
            ++i;
        }
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
