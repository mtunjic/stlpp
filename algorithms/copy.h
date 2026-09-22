#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Copy `n` elements from `first` to `dest`. Returns `{in, out}` past the
/// last copied positions. `n <= 0` is a no-op. Precondition: at least `n`
/// readable elements unless a sentinel overload is used.
///
/// Example:
///   std::vector<int> src{1, 2, 3, 4};
///   std::vector<int> dst(2);
///   mt::copy_n(src.begin(), 2, dst.begin());  // dst == {1, 2}
template <std::input_iterator I, std::weakly_incrementable O, Integer N>
    requires std::indirectly_copyable<I, O>
[[nodiscard]] constexpr std::pair<I, O> copy_n(I first, N n, O dest)
{
    while (n > N{0}) {
        *dest = *first;
        ++first;
        ++dest;
        --n;
    }
    return {first, dest};
}

/// Copy at most `n` elements of `[first, last)` to `dest`.
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O, Integer N>
    requires std::indirectly_copyable<I, O>
[[nodiscard]] constexpr std::pair<I, O> copy_n(I first, S last, N n, O dest)
{
    while (n > N{0} && first != last) {
        *dest = *first;
        ++first;
        ++dest;
        --n;
    }
    return {first, dest};
}

template <std::ranges::input_range R, std::weakly_incrementable O, Integer N>
    requires std::indirectly_copyable<std::ranges::iterator_t<R>, O>
[[nodiscard]] constexpr auto copy_n(R& range, N n, O dest)
{
    return copy_n(std::ranges::begin(range), std::ranges::end(range), n, dest);
}

/// Copy `n` elements through `buffer` (useful when source and dest overlap).
///
/// Example:
///   int buf[3];
///   auto [in, out] = mt::copy_n_with_buffer(src.begin(), 3, buf, dest.begin());
template <std::input_iterator I, Integer N, std::forward_iterator B, std::weakly_incrementable O>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, O>
[[nodiscard]] constexpr std::pair<I, O> copy_n_with_buffer(I first, N n, B buffer, O dest)
{
    auto [in, buf_end] = mt::copy_n(first, n, buffer);
    auto [buf_again, out] = mt::copy_n(buffer, buf_end, n, dest);
    (void)buf_again;
    return {in, out};
}

template <std::input_iterator I, std::sentinel_for<I> S, Integer N, std::forward_iterator B,
          std::weakly_incrementable O>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, O>
[[nodiscard]] constexpr std::pair<I, O> copy_n_with_buffer(I first, S last, N n, B buffer, O dest)
{
    auto [in, buf_end] = mt::copy_n(first, last, n, buffer);
    auto [buf_again, out] = mt::copy_n(buffer, buf_end, n, dest);
    (void)buf_again;
    return {in, out};
}

/// Swap `n` corresponding elements of two ranges. Returns iterators past the
/// last swapped pair.
///
/// Example:
///   std::vector<int> a{1, 2, 3};
///   std::vector<int> b{7, 8, 9};
///   mt::swap_ranges_n(a.begin(), b.begin(), 2);
///   // a == {7, 8, 3}, b == {1, 2, 9}
template <std::forward_iterator I0, std::forward_iterator I1, Integer N>
[[nodiscard]] constexpr std::pair<I0, I1> swap_ranges_n(I0 first0, I1 first1, N n)
{
    while (n > N{0}) {
        std::iter_swap(first0, first1);
        ++first0;
        ++first1;
        --n;
    }
    return {first0, first1};
}

template <std::forward_iterator I0, std::sentinel_for<I0> S0, std::forward_iterator I1,
          std::sentinel_for<I1> S1, Integer N>
[[nodiscard]] constexpr std::pair<I0, I1> swap_ranges_n(I0 first0, S0 last0, I1 first1, S1 last1,
                                                        N n)
{
    while (n > N{0} && first0 != last0 && first1 != last1) {
        std::iter_swap(first0, first1);
        ++first0;
        ++first1;
        --n;
    }
    return {first0, first1};
}

/// Copy `[first, last)` backward into the range ending at `result` (EoP ch. 9).
/// Returns the new begin of the written range. Safe for overlapping ranges
/// when the destination starts to the right of `first` (or equals the source).
///
/// Example:
///   std::vector<int> v{1, 2, 3, 0, 0};
///   mt::copy_backward(v.begin(), v.begin() + 3, v.end());  // {1, 2, 1, 2, 3}
template <std::bidirectional_iterator I, std::bidirectional_iterator O>
    requires std::indirectly_copyable<I, O>
[[nodiscard]] constexpr O copy_backward(I first, I last, O result)
{
    while (first != last) {
        *--result = *--last;
    }
    return result;
}

/// Counted backward copy of `n` elements ending at `last` into the range ending at `result`.
/// Returns `{new_last, new_result}` after stepping `n` predecessors.
template <std::bidirectional_iterator I, std::bidirectional_iterator O, Integer N>
    requires std::indirectly_copyable<I, O>
[[nodiscard]] constexpr std::pair<I, O> copy_backward_n(I last, N n, O result)
{
    while (n > N{0}) {
        *--result = *--last;
        --n;
    }
    return {last, result};
}

/// Copy `[first, last)` in reverse into the range ending at `result`.
///
/// Example:
///   std::vector<int> src{1, 2, 3};
///   std::vector<int> dst(3);
///   mt::reverse_copy_backward(src.begin(), src.end(), dst.end());  // dst == {3, 2, 1}
template <std::input_iterator I, std::sentinel_for<I> S, std::bidirectional_iterator O>
    requires std::indirectly_copyable<I, O>
[[nodiscard]] constexpr O reverse_copy_backward(I first, S last, O result)
{
    while (first != last) {
        *--result = *first;
        ++first;
    }
    return result;
}

/// Combine two ranges into one by selecting from the left or right under
/// predicate `use_left`, writing backward into `result` (EoP).
template <std::bidirectional_iterator I0, std::bidirectional_iterator I1,
          std::bidirectional_iterator O, typename P>
    requires std::predicate<P&, std::iter_reference_t<I0>, std::iter_reference_t<I1>>
[[nodiscard]] O combine_copy_backward(I0 f0, I0 l0, I1 f1, I1 l1, O result, P use_left)
{
    while (f0 != l0 || f1 != l1) {
        if (f0 == l0) {
            *--result = *--l1;
        } else if (f1 == l1) {
            *--result = *--l0;
        } else if (std::invoke(use_left, *std::prev(l0), *std::prev(l1))) {
            *--result = *--l0;
        } else {
            *--result = *--l1;
        }
    }
    return result;
}

/// Merge two sorted ranges backward into `result` (EoP `merge_copy_backward`).
template <std::bidirectional_iterator I0, std::bidirectional_iterator I1,
          std::bidirectional_iterator O, typename Comp = std::less<>>
[[nodiscard]] O merge_copy_backward(I0 f0, I0 l0, I1 f1, I1 l1, O result, Comp comp = {})
{
    return combine_copy_backward(f0, l0, f1, l1, result,
                                 [&](const auto& a, const auto& b) { return comp(b, a); });
}

}  // namespace mt
