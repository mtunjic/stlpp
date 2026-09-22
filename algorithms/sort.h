#pragma once

#include "integer.h"
#include "iterator_ops.h"
#include "partition.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// First iterator in a counted sorted range with `!(*it < value)` false for
/// upper bound: first element greater than `value`.
///
/// Example:
///   std::vector<int> v{1, 2, 2, 4};
///   auto u = mt::upper_bound_n(v.begin(), 4, 2);  // *u == 4
template <std::forward_iterator I, Integer N, typename T, typename Comp = std::less<>>
I upper_bound_n(I first, N n, const T& value, Comp comp = {})
{
    while (n > N{0}) {
        const N h = half(n);
        I middle = first;
        std::advance(middle, static_cast<Difference_type<I>>(h));
        if (!comp(value, *middle)) {
            first = ++middle;
            n -= h + N{1};
        } else {
            n = h;
        }
    }
    return first;
}

/// First iterator in a counted sorted range with `!(*it < value)`: first
/// element not less than `value`.
///
/// Example:
///   std::vector<int> v{1, 2, 2, 4};
///   auto l = mt::lower_bound_n(v.begin(), 4, 2);  // *l == 2
template <std::forward_iterator I, Integer N, typename T, typename Comp = std::less<>>
I lower_bound_n(I first, N n, const T& value, Comp comp = {})
{
    while (n > N{0}) {
        const N h = half(n);
        I middle = first;
        std::advance(middle, static_cast<Difference_type<I>>(h));
        if (comp(*middle, value)) {
            first = ++middle;
            n -= h + N{1};
        } else {
            n = h;
        }
    }
    return first;
}

template <std::forward_iterator I, std::sentinel_for<I> S, Integer N, typename T,
          typename Comp = std::less<>>
I upper_bound_n(I first, S last, N n, const T& value, Comp comp = {})
{
    return upper_bound_n(first, bounded_count(first, last, n), value, std::move(comp));
}

template <std::forward_iterator I, std::sentinel_for<I> S, Integer N, typename T,
          typename Comp = std::less<>>
I lower_bound_n(I first, S last, N n, const T& value, Comp comp = {})
{
    return lower_bound_n(first, bounded_count(first, last, n), value, std::move(comp));
}

/// Subrange of sorted elements equivalent to `value` (EoP ex. 6.7).
///
/// Example:
///   std::vector<int> v{1, 2, 2, 4};
///   auto [lo, hi] = mt::equal_range(v.begin(), v.end(), 2);
///   // distance(lo, hi) == 2
template <std::forward_iterator I, std::sentinel_for<I> S, typename T, typename Comp = std::less<>>
[[nodiscard]] std::pair<I, I> equal_range(I first, S last, const T& value, Comp comp = {})
{
    first = std::ranges::lower_bound(first, last, value, comp);
    return {first, std::ranges::upper_bound(first, last, value, comp)};
}

template <std::forward_iterator I, Integer N, typename T, typename Comp = std::less<>>
[[nodiscard]] std::pair<I, I> equal_range_n(I first, N n, const T& value, Comp comp = {})
{
    I lo = lower_bound_n(first, n, value, comp);
    const N used = static_cast<N>(std::distance(first, lo));
    const N rem = n - used;
    I hi = upper_bound_n(lo, rem, value, comp);
    return {lo, hi};
}

template <std::forward_iterator I, std::sentinel_for<I> S, Integer N, typename T,
          typename Comp = std::less<>>
[[nodiscard]] std::pair<I, I> equal_range_n(I first, S last, N n, const T& value, Comp comp = {})
{
    return equal_range_n(first, bounded_count(first, last, n), value, std::move(comp));
}

/// Insertion sort of `n` elements using binary search for the insertion point
/// (Stepanov).
///
/// Example:
///   std::vector<int> v{3, 1, 4, 1};
///   mt::binary_insertion_sort_n(v.begin(), 4);
///   // v == {1, 1, 3, 4}
template <std::random_access_iterator I, Integer N, typename Comp = std::less<>>
void binary_insertion_sort_n(I first, N n, Comp comp = {})
{
    if (n <= N{0}) {
        return;
    }
    for (N i = 1; i < n; ++i) {
        std::iter_value_t<I> value = std::move(first[static_cast<Difference_type<I>>(i)]);
        I insertion = upper_bound_n(first, i, value, comp);
        std::move_backward(insertion, first + i, first + i + 1);
        *insertion = std::move(value);
    }
}

template <std::random_access_iterator I, std::sentinel_for<I> S, Integer N,
          typename Comp = std::less<>>
void binary_insertion_sort_n(I first, S last, N n, Comp comp = {})
{
    binary_insertion_sort_n(first, bounded_count(first, last, n), std::move(comp));
}

/// Make `[first1, last1)` the sorted subrange of `[first0, last0)` that
/// belongs in those positions (nth_element + partial_sort).
///
/// Example:
///   std::vector<int> v{9, 1, 8, 2, 7, 3};
///   mt::sort_subrange(v.begin(), v.end(), v.begin() + 1, v.begin() + 4);
template <std::random_access_iterator I>
void sort_subrange(I first0, I last0, I first1, I last1)
{
    if (first1 == last1) {
        return;
    }
    if (first1 != first0) {
        std::nth_element(first0, first1, last0);
        ++first1;
    }
    std::partial_sort(first1, last1, last0);
}

/// Write every pairwise sum `a[i] + b[j]` into `dest`, then sort that sequence
/// (Lambert: sorting the Minkowski sum of two sequences). Extra memory O(|a|*|b|).
///
/// Example:
///   std::vector<int> a{1, 3}, b{0, 10};
///   std::vector<int> sums;
///   mt::sort_sums(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(sums));
///   // sums == {1, 3, 11, 13}
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O>
O sort_sums(I1 a, S1 a_last, I2 b, S2 b_last, O dest)
{
    using Sum = decltype(*a + *b);
    std::vector<Value_type<I2>> b_copy;
    std::ranges::copy(b, b_last, std::back_inserter(b_copy));
    std::vector<Sum> sums;
    for (; a != a_last; ++a) {
        for (const auto& y : b_copy) {
            sums.push_back(*a + y);
        }
    }
    std::sort(sums.begin(), sums.end());
    return std::copy(sums.begin(), sums.end(), dest);
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
[[nodiscard]] auto sort_sums(R1&& a, R2&& b)
{
    using Sum = decltype(*std::ranges::begin(a) + *std::ranges::begin(b));
    std::vector<Sum> out;
    sort_sums(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
              std::ranges::end(b), std::back_inserter(out));
    return out;
}

/// Full sort by repeatedly placing the median of the remaining subrange
/// (`std::nth_element`) and recursing on both sides. Small tails use
/// `binary_insertion_sort_n`.
///
/// Example:
///   std::vector<int> v{9, 1, 8, 2, 7};
///   mt::median_sort(v.begin(), v.end());  // {1, 2, 7, 8, 9}
template <std::random_access_iterator I, typename Comp = std::less<>>
void median_sort(I first, I last, Comp comp = {})
{
    const auto n = last - first;
    if (n < 2) {
        return;
    }
    if (n < 16) {
        binary_insertion_sort_n(first, static_cast<Difference_type<I>>(n), comp);
        return;
    }
    const I mid = first + n / 2;
    std::nth_element(first, mid, last, comp);
    median_sort(first, mid, comp);
    median_sort(mid + 1, last, comp);
}

template <std::ranges::random_access_range R, typename Comp = std::less<>>
void median_sort(R&& range, Comp comp = {})
{
    median_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
}

}  // namespace mt
