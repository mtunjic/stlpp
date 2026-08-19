#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// First iterator where `pred` fails (Haskell `span` split point).
///
/// Example:
///   std::vector<int> v{2, 4, 5, 6};
///   auto m = mt::span_if(v, [](int x) { return x % 2 == 0; });
///   // [v.begin(), m) == {2, 4}
template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] I span_if(I first, S last, P pred)
{
    return std::find_if_not(first, last, std::move(pred));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] auto span_if(R&& range, P pred)
{
    return span_if(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

/// First iterator where `pred` holds (Haskell `break`).
template <std::input_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] I break_if(I first, S last, P pred)
{
    return std::find_if(first, last, std::move(pred));
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] auto break_if(R&& range, P pred)
{
    return break_if(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

/// Advance at most `n` steps; return the split iterator (Haskell `splitAt`).
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   auto m = mt::split_at(v.begin(), v.end(), 2);  // *m == 3
template <std::input_iterator I, std::sentinel_for<I> S, Integer N>
[[nodiscard]] I split_at(I first, S last, N n)
{
    while (n > N{0} && first != last) {
        ++first;
        --n;
    }
    return first;
}

template <std::ranges::input_range R, Integer N>
[[nodiscard]] auto split_at(R&& range, N n)
{
    return split_at(std::ranges::begin(range), std::ranges::end(range), n);
}

}  // namespace mt
