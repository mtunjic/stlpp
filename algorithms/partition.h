#pragma once

#include "integer.h"
#include "iterator_ops.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

namespace mt {

/// Slide `[first, last)` so that it occupies a window ending/starting at `p`
/// (Sean Parent). Returns the new bounds of the slid range.
///
/// Example:
///   std::vector<int> v{0, 1, 2, 3, 4, 5};
///   mt::slide(v.begin() + 1, v.begin() + 3, v.begin() + 5);
template <std::random_access_iterator I>
[[nodiscard]] std::pair<I, I> slide(I first, I last, I p)
{
    if (p < first) {
        return {p, std::rotate(p, first, last)};
    }
    if (last < p) {
        return {std::rotate(first, last, p), p};
    }
    return {first, last};
}

/// Gather every element satisfying `pred` around `pos` (Marshall Clow).
/// Returns `{first_kept, last_kept}` of the gathered run.
///
/// Example:
///   std::array<int, 6> a{9, 0, 1, 4, 0, 7};
///   mt::gather(a.begin(), a.end(), a.begin() + 3, [](int x) { return x < 2; });
template <std::bidirectional_iterator I, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] std::pair<I, I> gather(I first, I last, I pos, P pred)
{
    return {
        std::stable_partition(first, pos, std::not_fn(pred)),
        std::stable_partition(pos, last, pred)};
}

/// Stable-partition by *position* predicate `p(it)` rather than `p(*it)`
/// (Sean Parent). Requires random-access.
///
/// Example:
///   int a[] = {1, 2, 3, 4};
///   bool mask[] = {0, 1, 0, 1};
///   auto mid = mt::stable_partition_position(std::begin(a), std::end(a),
///       [&](auto it) { return mask[it - std::begin(a)]; });
///   // even-index values first: 2 4 ^ 1 3
template <std::random_access_iterator I, typename P>
    requires std::predicate<P&, I>
I stable_partition_position(I first, I last, P p)
{
    const auto n = last - first;
    if (n == 0) {
        return first;
    }
    if (n == 1) {
        return first + static_cast<Difference_type<I>>(std::invoke(p, first) ? 1 : 0);
    }
    const I middle = first + half(n);
    return std::rotate(stable_partition_position(first, middle, p), middle,
                       stable_partition_position(middle, last, p));
}

/// Binary search on a counted partitioned range (Stepanov).
/// Precondition: `[first, first+n)` is partitioned by `pred`.
///
/// Example:
///   std::vector<int> v{1, 3, 5, 0, 2};  // partitioned by x < 4? no —
///   // use a true partition: {1, 2, 3, 8, 9} pred x < 5 -> point at 8
template <std::forward_iterator I, Integer N, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
I partition_point_n(I first, N n, P pred)
{
    while (n > N{0}) {
        const N h = half(n);
        I middle = first;
        std::advance(middle, static_cast<Difference_type<I>>(h));
        if (std::invoke(pred, *middle)) {
            first = ++middle;
            n -= h + N{1};
        } else {
            n = h;
        }
    }
    return first;
}

template <std::forward_iterator I, std::sentinel_for<I> S, Integer N, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
I partition_point_n(I first, S last, N n, P pred)
{
    return partition_point_n(first, bounded_count(first, last, n), std::move(pred));
}

}  // namespace mt
