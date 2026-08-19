#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Assign `*first = f(*first)` for every element in `[first, last)`.
///
/// Example:
///   std::vector<int> v{1, 2, 3};
///   mt::apply(v, [](int x) { return x * x; });
///   // v == {1, 4, 9}
template <std::forward_iterator I, std::sentinel_for<I> S, typename F>
    requires std::invocable<F&, std::iter_reference_t<I>> &&
             std::indirectly_writable<I, std::invoke_result_t<F&, std::iter_reference_t<I>>>
void apply(I first, S last, F f)
{
    for (; first != last; ++first) {
        *first = std::invoke(f, *first);
    }
}

template <std::ranges::forward_range R, typename F>
void apply(R&& range, F f)
{
    apply(std::ranges::begin(range), std::ranges::end(range), std::move(f));
}

/// Apply `f` until `pred(*i)` is true (or the range ends). Returns the stop
/// iterator. The stopping element is not transformed.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   mt::apply_until(v, [](int x) { return x + 10; },
///                   [](int x) { return x == 3; });
///   // v == {11, 12, 3, 4}
template <std::forward_iterator I, std::sentinel_for<I> S, typename F, typename P>
    requires std::invocable<F&, std::iter_reference_t<I>> &&
             std::predicate<P&, std::iter_reference_t<I>>
I apply_until(I first, S last, F f, P pred)
{
    for (; first != last && !std::invoke(pred, *first); ++first) {
        *first = std::invoke(f, *first);
    }
    return first;
}

template <std::ranges::forward_range R, typename F, typename P>
auto apply_until(R&& range, F f, P pred)
{
    return apply_until(std::ranges::begin(range), std::ranges::end(range),
                       std::move(f), std::move(pred));
}

}  // namespace mt
