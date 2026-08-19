#pragma once

#include "parallel.h"

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Sequential left fold: `op(op(init, a0), a1), ...`.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   mt::fold(v, 0, std::plus<>{});  // 10
template <std::input_iterator I, std::sentinel_for<I> S, typename T, typename Op>
[[nodiscard]] T fold(I first, S last, T init, Op op)
{
    for (; first != last; ++first) {
        init = std::invoke(op, std::move(init), *first);
    }
    return init;
}

template <std::ranges::input_range R, typename T, typename Op>
[[nodiscard]] auto fold(R&& range, T init, Op op)
{
    return fold(std::ranges::begin(range), std::ranges::end(range), std::move(init),
                std::move(op));
}

/// Parallel reduction. `op` must be associative (and should be commutative).
/// Sequential below `par::default_cutoff`.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   mt::reduce(v, 0, std::plus<>{});  // 10
template <std::random_access_iterator I, std::sentinel_for<I> S, typename T, typename Op>
[[nodiscard]] T reduce(I first, S last, T init, Op op)
{
    using D = std::iter_difference_t<I>;
    const D n = last - first;
    if (n <= 0) {
        return init;
    }
    const unsigned t = par::thread_count();
    if (static_cast<std::size_t>(n) <= par::default_cutoff || t <= 1) {
        return fold(first, last, std::move(init), std::move(op));
    }

    const unsigned chunks = std::min<unsigned>(t, static_cast<unsigned>(n));
    const D chunk = n / static_cast<D>(chunks);
    std::vector<T> parts(chunks);
    par::for_index(chunks, [&](std::size_t c) {
        I b = first + static_cast<D>(c) * chunk;
        I e = (c + 1 == chunks) ? last : b + chunk;
        T acc = *b;
        ++b;
        parts[c] = fold(b, e, std::move(acc), op);
    });
    T acc = std::move(init);
    for (T& p : parts) {
        acc = std::invoke(op, std::move(acc), std::move(p));
    }
    return acc;
}

template <std::ranges::random_access_range R, typename T, typename Op>
[[nodiscard]] auto reduce(R&& range, T init, Op op)
{
    return reduce(std::ranges::begin(range), std::ranges::end(range), std::move(init),
                  std::move(op));
}

template <std::random_access_iterator I, std::sentinel_for<I> S>
[[nodiscard]] auto reduce(I first, S last)
{
    using T = std::iter_value_t<I>;
    return reduce(first, last, T{}, std::plus<>{});
}

template <std::ranges::random_access_range R>
[[nodiscard]] auto reduce(R&& range)
{
    return reduce(std::ranges::begin(range), std::ranges::end(range));
}

}  // namespace mt
