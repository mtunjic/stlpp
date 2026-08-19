#pragma once

#include "concepts.h"
#include "parallel.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Adaptive parallel sort: `std::sort` below `par::default_cutoff`, otherwise
/// chunk-sort + pairwise `inplace_merge`.
///
/// Example:
///   std::vector<int> v{3, 1, 4, 1, 5};
///   mt::psort(v);
///   // v == {1, 1, 3, 4, 5}
template <std::random_access_iterator I, typename Comp = std::less<>>
void psort(I first, I last, Comp comp = {})
{
    using D = std::iter_difference_t<I>;
    const D n = last - first;
    if (n < 2) {
        return;
    }
    const unsigned t = par::thread_count();
    if (static_cast<std::size_t>(n) < par::default_cutoff || t <= 1) {
        std::sort(first, last, comp);
        return;
    }

    const unsigned chunks = std::min<unsigned>(t, static_cast<unsigned>(n));
    const D chunk = n / static_cast<D>(chunks);
    std::vector<I> cuts(chunks + 1);
    cuts[0] = first;
    for (unsigned i = 0; i < chunks; ++i) {
        cuts[i + 1] = (i + 1 == chunks) ? last : cuts[i] + chunk;
    }

    par::for_index(chunks, [&](std::size_t i) {
        std::sort(cuts[i], cuts[i + 1], comp);
    });

    for (unsigned width = 1; width < chunks; width *= 2) {
        const unsigned stride = width * 2;
        const unsigned jobs = (chunks + stride - 1) / stride;
        par::for_index(jobs, [&](std::size_t j) {
            const unsigned lo = static_cast<unsigned>(j) * stride;
            const unsigned mid = lo + width;
            if (mid >= chunks) {
                return;
            }
            const unsigned hi = std::min(lo + stride, chunks);
            std::inplace_merge(cuts[lo], cuts[mid], cuts[hi], comp);
        });
    }
}

template <std::ranges::random_access_range R, typename Comp = std::less<>>
void psort(R&& range, Comp comp = {})
{
    psort(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
}

/// Parallel bucket sort for arithmetic values: histogram into B buckets, sort
/// each bucket, concatenate. Dual-GPU offload is not implemented (no CUDA/HIP
/// in this tree); buckets run on CPU threads instead.
///
/// Example:
///   std::vector<int> v{9, 1, 8, 2, 7};
///   mt::partial_bucketsort(v);
///   // v == {1, 2, 7, 8, 9}
template <std::random_access_iterator I>
    requires Number<std::iter_value_t<I>>
void partial_bucketsort(I first, I last)
{
    using T = std::iter_value_t<I>;
    const auto n = static_cast<std::size_t>(last - first);
    if (n < 2) {
        return;
    }
    if (n < par::default_cutoff || par::thread_count() <= 1) {
        std::sort(first, last);
        return;
    }

    auto [lo_it, hi_it] = std::minmax_element(first, last);
    const T lo = *lo_it;
    const T hi = *hi_it;
    if (lo == hi) {
        return;
    }

    const std::size_t B =
        std::min(n, std::max<std::size_t>(static_cast<std::size_t>(par::thread_count()) * 8, 16));
    const double dlo = static_cast<double>(lo);
    const double span = static_cast<double>(hi) - dlo;
    auto bucket_of = [dlo, span, B](T v) -> std::size_t {
        const double t = (static_cast<double>(v) - dlo) / span;
        auto i = static_cast<std::size_t>(t * static_cast<double>(B - 1));
        return i < B ? i : B - 1;
    };

    const unsigned chunks = std::min<unsigned>(par::thread_count(), static_cast<unsigned>(n));
    using D = std::iter_difference_t<I>;
    const D chunk = static_cast<D>(n) / static_cast<D>(chunks);
    std::vector<std::vector<std::vector<T>>> local(chunks, std::vector<std::vector<T>>(B));

    par::for_index(chunks, [&](std::size_t c) {
        I b = first + static_cast<D>(c) * chunk;
        I e = (c + 1 == chunks) ? last : b + chunk;
        auto& mine = local[c];
        for (; b != e; ++b) {
            mine[bucket_of(*b)].push_back(*b);
        }
    });

    std::vector<std::vector<T>> buckets(B);
    for (std::size_t k = 0; k < B; ++k) {
        std::size_t total = 0;
        for (unsigned c = 0; c < chunks; ++c) {
            total += local[c][k].size();
        }
        buckets[k].reserve(total);
        for (unsigned c = 0; c < chunks; ++c) {
            auto& src = local[c][k];
            buckets[k].insert(buckets[k].end(), src.begin(), src.end());
        }
    }

    par::for_index(B, [&](std::size_t k) { std::sort(buckets[k].begin(), buckets[k].end()); });

    I out = first;
    for (auto& bkt : buckets) {
        out = std::move(bkt.begin(), bkt.end(), out);
    }
}

template <std::ranges::random_access_range R>
    requires Number<std::ranges::range_value_t<R>>
void partial_bucketsort(R&& range)
{
    partial_bucketsort(std::ranges::begin(range), std::ranges::end(range));
}

}  // namespace mt
