#pragma once

#include "binary_counter.h"
#include "parallel.h"

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Sequential left fold: `op(op(init, a0), a1), ...`. Always left-associated.
/// Use this when `op` is not associative.
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
    return fold(std::ranges::begin(range), std::ranges::end(range), std::move(init), std::move(op));
}

/// Reduce a non-empty range with no identity (EoP `reduce_nonempty`).
/// Precondition: `first != last`.
template <std::input_iterator I, std::sentinel_for<I> S, typename Op>
[[nodiscard]] auto reduce_nonempty(I first, S last, Op op)
{
    auto acc = *first;
    ++first;
    return fold(first, last, std::move(acc), std::move(op));
}

template <std::ranges::input_range R, typename Op>
[[nodiscard]] auto reduce_nonempty(R&& range, Op op)
{
    return reduce_nonempty(std::ranges::begin(range), std::ranges::end(range), std::move(op));
}

/// Parallel reduction of a random-access range.
/// `op` **must be associative** (and should be commutative): chunks are folded
/// independently and then combined, which is not a strict left fold. Use
/// `mt::fold` for non-associative operators. Sequential below
/// `par::default_cutoff`.
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

    const unsigned chunks = par::worker_chunks(static_cast<std::size_t>(n));
    if (chunks == 0) {
        return init;
    }
    std::vector<T> parts(chunks);
    par::for_index(chunks, [&](std::size_t c) {
        auto [b, e] =
            par::chunk_bounds(first, static_cast<std::size_t>(n), static_cast<unsigned>(c), chunks);
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

/// Sequential reduce for non-random-access iterators (always a left fold).
template <std::input_iterator I, std::sentinel_for<I> S, typename T, typename Op>
    requires(!std::random_access_iterator<I>)
[[nodiscard]] T reduce(I first, S last, T init, Op op)
{
    return fold(first, last, std::move(init), std::move(op));
}

template <std::ranges::random_access_range R, typename T, typename Op>
[[nodiscard]] auto reduce(R&& range, T init, Op op)
{
    return reduce(std::ranges::begin(range), std::ranges::end(range), std::move(init),
                  std::move(op));
}

template <std::ranges::input_range R, typename T, typename Op>
    requires(!std::ranges::random_access_range<R>)
[[nodiscard]] auto reduce(R&& range, T init, Op op)
{
    return fold(std::ranges::begin(range), std::ranges::end(range), std::move(init), std::move(op));
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

/// Balanced binary-counter reduce (EoP). Prefer for associative ops when a
/// more balanced association tree than left-fold is desired.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   mt::reduce_balanced(v.begin(), v.end(), std::plus<>{}, 0);  // 10
template <std::input_iterator I, std::sentinel_for<I> S, typename Op, typename T>
[[nodiscard]] T reduce_balanced(I first, S last, Op op, T zero)
{
    binary_counter<T, Op> c(std::move(op), std::move(zero));
    for (; first != last; ++first) {
        c.add(*first);
    }
    return c.reduce();
}

template <std::ranges::input_range R, typename Op, typename T>
[[nodiscard]] auto reduce_balanced(R&& range, Op op, T zero)
{
    return reduce_balanced(std::ranges::begin(range), std::ranges::end(range), std::move(op),
                           std::move(zero));
}

}  // namespace mt
