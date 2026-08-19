#pragma once

#include "concepts.h"

#include <functional>
#include <iterator>
#include <utility>

namespace mt {

/// Iterators to the minimum and maximum in `[first, last)` (Stepanov & Pohl).
/// Empty range: `{first, first}`. One element: both point at it.
///
/// Example:
///   std::vector<int> v{3, 1, 4, 1, 5};
///   auto [lo, hi] = mt::min_max(v.begin(), v.end());
///   // *lo == 1, *hi == 5
template <std::forward_iterator I, typename Comp = std::less<>>
[[nodiscard]] constexpr std::pair<I, I> min_max(I first, I last, Comp comp = {})
{
    if (first == last) {
        return {first, first};
    }
    I min_it = first;
    I max_it = first;
    ++first;
    if (first == last) {
        return {min_it, max_it};
    }

    if (comp(*first, *min_it)) {
        min_it = first;
    } else {
        max_it = first;
    }
    ++first;

    while (first != last) {
        I next = first;
        ++next;
        if (next == last) {
            if (comp(*first, *min_it)) {
                min_it = first;
            } else if (!comp(*first, *max_it)) {
                max_it = first;
            }
            break;
        }

        I a = first;
        I b = next;
        if (comp(*b, *a)) {
            std::swap(a, b);
        }
        if (comp(*a, *min_it)) {
            min_it = a;
        }
        if (!comp(*b, *max_it)) {
            max_it = b;
        }
        ++next;
        first = next;
    }
    return {min_it, max_it};
}

}  // namespace mt
