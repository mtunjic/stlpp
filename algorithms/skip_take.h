#pragma once

#include "concepts.h"

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Drop the first `n` elements; return the resulting iterator.
///
/// Example:
///   auto i = mt::skip(v.begin(), v.end(), 2);
template <std::input_iterator I, std::sentinel_for<I> S>
[[nodiscard]] constexpr I skip(I first, S last, Difference_type<I> n)
{
    while (n > 0 && first != last) {
        ++first;
        --n;
    }
    return first;
}

template <std::ranges::input_range R>
[[nodiscard]] auto skip(R& range, std::ranges::range_difference_t<R> n)
{
    auto b = skip(std::ranges::begin(range), std::ranges::end(range), n);
    return std::ranges::subrange(b, std::ranges::end(range));
}

/// Skip while `pred` holds.
///
/// Example:
///   auto i = mt::skip_while(v.begin(), v.end(), [](int x) { return x < 0; });
template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] constexpr I skip_while(I first, S last, P pred)
{
    while (first != last && std::invoke(pred, *first)) {
        ++first;
    }
    return first;
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] auto skip_while(R& range, P pred)
{
    auto b = skip_while(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
    return std::ranges::subrange(b, std::ranges::end(range));
}

/// Take at most `n` elements as `[first, taken)`.
///
/// Example:
///   auto [a, b] = mt::take(v.begin(), v.end(), 3);
template <std::forward_iterator I, std::sentinel_for<I> S>
[[nodiscard]] constexpr std::pair<I, I> take(I first, S last, Difference_type<I> n)
{
    I taken = first;
    while (n > 0 && taken != last) {
        ++taken;
        --n;
    }
    return {first, taken};
}

template <std::ranges::forward_range R>
[[nodiscard]] auto take(R& range, std::ranges::range_difference_t<R> n)
{
    auto [a, b] = take(std::ranges::begin(range), std::ranges::end(range), n);
    return std::ranges::subrange(a, b);
}

/// Take while `pred` holds.
///
/// Example:
///   std::vector<int> v{-1, -2, 3, 4};
///   auto [a, b] = mt::take_while(v.begin(), v.end(), [](int x) { return x < 0; });
///   // [a, b) == {-1, -2}
template <std::forward_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] constexpr std::pair<I, I> take_while(I first, S last, P pred)
{
    I taken = first;
    while (taken != last && std::invoke(pred, *taken)) {
        ++taken;
    }
    return {first, taken};
}

template <std::ranges::forward_range R, typename P>
[[nodiscard]] auto take_while(R& range, P pred)
{
    auto [a, b] = take_while(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
    return std::ranges::subrange(a, b);
}

}  // namespace mt
