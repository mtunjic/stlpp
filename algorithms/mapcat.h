#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Map then flatten one level (Lisp `mapcan`, Haskell `concatMap`).
///
/// Example:
///   auto dup = [](int x) { return std::vector{x, x}; };
///   mt::mapcat(std::vector{1, 2}, dup);  // {1, 1, 2, 2}
template <std::input_iterator I, std::sentinel_for<I> S, typename F, std::weakly_incrementable O>
O mapcat(I first, S last, F f, O dest)
{
    for (; first != last; ++first) {
        auto sub = std::invoke(f, *first);
        dest = std::ranges::copy(sub, dest).out;
    }
    return dest;
}

template <std::ranges::input_range R, typename F>
[[nodiscard]] auto mapcat(R&& range, F f)
{
    using Sub = std::invoke_result_t<F&, std::ranges::range_reference_t<R>>;
    using T = std::ranges::range_value_t<Sub>;
    std::vector<T> out;
    mapcat(std::ranges::begin(range), std::ranges::end(range), std::move(f),
           std::back_inserter(out));
    return out;
}

/// Flatten one level of nested ranges into `dest`.
///
/// Example:
///   mt::flatten(std::vector<std::vector<int>>{{1, 2}, {3}});  // {1, 2, 3}
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O>
O flatten(I first, S last, O dest)
{
    for (; first != last; ++first) {
        auto&& inner = *first;
        dest = std::ranges::copy(inner, dest).out;
    }
    return dest;
}

template <std::ranges::input_range R>
[[nodiscard]] auto flatten(R&& range)
{
    using Inner = std::ranges::range_value_t<R>;
    std::vector<std::ranges::range_value_t<Inner>> out;
    flatten(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out));
    return out;
}

}  // namespace mt
