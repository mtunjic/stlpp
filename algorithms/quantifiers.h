#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>

namespace mt {

/// Lisp `every` / Haskell `all`.
///
/// Example:
///   mt::all(std::vector{2, 4, 6}, [](int x) { return x % 2 == 0; });  // true
template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] bool all(I first, S last, P pred)
{
    return std::ranges::all_of(first, last, std::move(pred));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] bool all(R&& range, P pred)
{
    return all(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

/// Lisp `some` / Haskell `any`.
template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] bool any(I first, S last, P pred)
{
    return std::ranges::any_of(first, last, std::move(pred));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] bool any(R&& range, P pred)
{
    return any(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

/// Lisp `notany`.
template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] bool none(I first, S last, P pred)
{
    return std::ranges::none_of(first, last, std::move(pred));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] bool none(R&& range, P pred)
{
    return none(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] std::iter_difference_t<I> count_if(I first, S last, P pred)
{
    return std::ranges::count_if(first, last, std::move(pred));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] auto count_if(R&& range, P pred)
{
    return count_if(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] std::iter_difference_t<I> count_if_not(I first, S last, P pred)
{
    return std::ranges::count_if(first, last, std::not_fn(std::move(pred)));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] auto count_if_not(R&& range, P pred)
{
    return count_if_not(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

template <std::input_iterator I, std::sentinel_for<I> S, typename T>
[[nodiscard]] std::iter_difference_t<I> count_not(I first, S last, const T& value)
{
    return std::ranges::count_if(first, last, [&](const auto& x) { return !(x == value); });
}

template <std::ranges::input_range R, typename T>
[[nodiscard]] auto count_not(R&& range, const T& value)
{
    return count_not(std::ranges::begin(range), std::ranges::end(range), value);
}

}  // namespace mt
