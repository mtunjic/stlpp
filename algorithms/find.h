#pragma once

#include "concepts.h"

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Walk at most `n` elements; return `{iterator, remaining}` where remaining
/// is 0 if the counted range was exhausted without a match.
///
/// Example:
///   std::vector<int> v{1, 3, 4, 6};
///   auto [it, left] = mt::find_if_n(v.begin(), 4, [](int x) { return x % 2 == 0; });
///   // *it == 4, left == 2
template <std::input_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] constexpr std::pair<I, Difference_type<I>>
find_if_n(I first, Difference_type<I> n, P pred)
{
    while (n && !std::invoke(pred, *first)) {
        ++first;
        --n;
    }
    return {first, n};
}

/// Last iterator in `[first, last)` that satisfies `pred`, or `last`.
///
/// Example:
///   std::vector<int> v{5, 3, 0, 1, 8};
///   auto p = mt::find_backward_if(v.begin(), v.end(),
///                                 [](int x) { return x == 0; });
///   // p != v.end() && *p == 0
template <std::bidirectional_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] constexpr I find_backward_if(I first, I last, P pred)
{
    const I end = last;
    while (first != last) {
        --last;
        if (std::invoke(pred, *last)) {
            return last;
        }
    }
    return end;
}

/// Counted backward search: look at the last `n` elements of `[first, last)`.
/// Not found returns `last` (the original end), not `first`.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4, 5};
///   auto p = mt::find_backward_if_n(v.begin(), v.end(), 2,
///                                   [](int x) { return x % 2 == 0; });
///   // *p == 4  (only 4 and 5 are examined)
template <std::bidirectional_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] constexpr I find_backward_if_n(I first, I last, Difference_type<I> n, P pred)
{
    const I end = last;
    while (n && first != last) {
        --last;
        --n;
        if (std::invoke(pred, *last)) {
            return last;
        }
    }
    return end;
}

/// Last element that does **not** satisfy `pred`, or `last`.
///
/// Example:
///   std::vector<int> v{2, 4, 5, 8};
///   auto p = mt::find_backward_if_not(v.begin(), v.end(),
///                                     [](int x) { return x % 2 == 0; });
///   // *p == 5
template <std::bidirectional_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] constexpr I find_backward_if_not(I first, I last, P pred)
{
    return find_backward_if(first, last, std::not_fn(std::move(pred)));
}

template <std::bidirectional_iterator I, typename T>
[[nodiscard]] constexpr I find_backward(I first, I last, const T& value)
{
    return find_backward_if(first, last, [&](const auto& x) { return x == value; });
}

/// Collect every iterator whose value equals `value`.
///
/// Example:
///   std::vector<int> v{1, 2, 1, 3};
///   auto hits = mt::find_all(v, 1);  // two iterators, both *hit == 1
template <std::ranges::forward_range R, typename T>
[[nodiscard]] auto find_all(R& range, const T& value)
{
    using It = std::ranges::iterator_t<R>;
    std::vector<It> hits;
    for (auto i = std::ranges::begin(range), last = std::ranges::end(range); i != last; ++i) {
        if (*i == value) {
            hits.push_back(i);
        }
    }
    return hits;
}

}  // namespace mt
