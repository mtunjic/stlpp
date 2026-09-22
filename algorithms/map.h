#pragma once

#include "parallel.h"

#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace mt {

/// Parallel transform of a random-access range into `dest`.
/// Sequential for non-random-access iterators.
///
/// Example:
///   std::vector<int> v{1, 2, 3}, out(3);
///   mt::map(v.begin(), v.end(), out.begin(), [](int x) { return x * x; });
///   // out == {1, 4, 9}
template <std::random_access_iterator I, std::sentinel_for<I> S, std::random_access_iterator O,
          typename F>
O map(I first, S last, O dest, F f)
{
    const I end = std::ranges::next(first, last);
    const auto n = end - first;
    if (n <= 0) {
        return dest;
    }
    if constexpr (!std::is_lvalue_reference_v<std::iter_reference_t<O>>) {
        for (; first != end; ++first, ++dest) {
            *dest = std::invoke(f, *first);
        }
        return dest;
    }
    const I origin = first;
    par::for_chunks(first, end, [f, dest, origin](I b, I e) mutable {
        O o = dest + (b - origin);
        for (; b != e; ++b, ++o) {
            *o = std::invoke(f, *b);
        }
    });
    return dest + n;
}

template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O, typename F>
    requires(!std::random_access_iterator<I> || !std::random_access_iterator<O>)
O map(I first, S last, O dest, F f)
{
    for (; first != last; ++first, ++dest) {
        *dest = std::invoke(f, *first);
    }
    return dest;
}

/// Parallel map that returns a new vector.
///
/// Example:
///   auto squares = mt::map(std::vector{1, 2, 3}, [](int x) { return x * x; });
///   // squares == {1, 4, 9}
template <std::ranges::input_range R, typename F>
[[nodiscard]] auto map(R&& range, F f)
{
    using U = std::remove_cvref_t<std::invoke_result_t<F&, std::ranges::range_reference_t<R>>>;
    std::vector<U> out;
    if constexpr (std::ranges::random_access_range<R> && std::ranges::sized_range<R> &&
                  std::default_initializable<U> && !std::same_as<U, bool>) {
        out.resize(std::ranges::size(range));
        map(std::ranges::begin(range), std::ranges::end(range), out.begin(), std::move(f));
    } else {
        for (auto&& x : range) {
            out.push_back(std::invoke(f, x));
        }
    }
    return out;
}

}  // namespace mt
