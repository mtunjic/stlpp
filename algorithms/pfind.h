#pragma once

#include "parallel.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <vector>

namespace mt {

/// Parallel `find_if`. Returns the leftmost match, or `last`.
/// Sequential below `par::default_cutoff`.
///
/// Example:
///   std::vector<int> v{1, 3, 4, 6};
///   auto it = mt::pfind_if(v, [](int x) { return x % 2 == 0; });
///   // *it == 4
template <std::random_access_iterator I, std::sentinel_for<I> S, typename P>
    requires std::predicate<P&, std::iter_reference_t<I>>
[[nodiscard]] I pfind_if(I first, S last, P pred)
{
    using D = std::iter_difference_t<I>;
    const D n = last - first;
    if (n <= 0) {
        return last;
    }
    const unsigned t = par::thread_count();
    if (static_cast<std::size_t>(n) <= par::default_cutoff || t <= 1) {
        return std::find_if(first, last, std::move(pred));
    }

    const unsigned chunks = par::worker_chunks(static_cast<std::size_t>(n));
    if (chunks == 0) {
        return last;
    }
    std::vector<I> hits(chunks, last);
    par::for_index(chunks, [&](std::size_t c) {
        auto [b, e] = par::chunk_bounds(first, static_cast<std::size_t>(n),
                                        static_cast<unsigned>(c), chunks);
        auto it = std::find_if(b, e, pred);
        if (it != e) {
            hits[c] = it;
        }
    });
    I best = last;
    for (I h : hits) {
        if (h != last && (best == last || h < best)) {
            best = h;
        }
    }
    return best;
}

template <std::ranges::random_access_range R, typename P>
[[nodiscard]] auto pfind_if(R& range, P pred)
{
    return pfind_if(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

/// Parallel `find`. Returns the leftmost match, or `last`.
///
/// Example:
///   std::vector<int> v{9, 1, 4, 1};
///   auto it = mt::pfind(v, 1);  // first 1
template <std::random_access_iterator I, std::sentinel_for<I> S, typename T>
[[nodiscard]] I pfind(I first, S last, const T& value)
{
    return pfind_if(first, last, [&](const auto& x) { return x == value; });
}

template <std::ranges::random_access_range R, typename T>
[[nodiscard]] auto pfind(R& range, const T& value)
{
    return pfind(std::ranges::begin(range), std::ranges::end(range), value);
}

}  // namespace mt
