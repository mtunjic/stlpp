#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>

namespace mt {

/// Stable-partition so that elements satisfying `pred` stay; the rest are
/// moved to the tail. Returns the new logical end (same as `std::remove_if`
/// of the negated predicate).
///
/// Example:
///   std::vector<int> v{9, 0, 1, 4, 7};
///   auto p = mt::remove_if_not(v.begin(), v.end(), [](int x) { return x < 5; });
///   v.erase(p, v.end());
///   // v == {0, 1, 4}
template <std::forward_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
I remove_if_not(I first, I last, P pred)
{
    return std::remove_if(first, last, std::not_fn(std::move(pred)));
}

template <std::ranges::forward_range R, typename P>
auto remove_if_not(R& range, P pred)
{
    return remove_if_not(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

/// Compact from the back: keep elements that satisfy `pred`, shifting them
/// toward `last`. Returns the new logical begin of the kept suffix.
/// Drop the discarded prefix with `erase(first, write)` or ignore `[first, write)`.
///
/// Example:
///   std::vector<int> v{9, 0, 1, 4, 7};
///   auto w = mt::remove_if_not_backward(v.begin(), v.end(),
///                                       [](int x) { return x < 5; });
///   // [w, v.end()) == {0, 1, 4}
template <std::bidirectional_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
I remove_if_not_backward(I first, I last, P pred)
{
    I write = last;
    while (first != last) {
        --last;
        if (std::invoke(pred, *last)) {
            --write;
            if (write != last) {
                *write = std::move(*last);
            }
        }
    }
    return write;
}

}  // namespace mt
