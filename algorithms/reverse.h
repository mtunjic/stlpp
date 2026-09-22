#pragma once

#include "copy.h"
#include "integer.h"
#include "iterator_ops.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Reverse `n` elements starting at `first` using only O(log n) extra
/// iterators (Stepanov). Works on forward iterators. Returns the iterator
/// one past the reversed range.
///
/// Example:
///   std::list<int> xs{1, 2, 3, 4};
///   mt::reverse_n(xs.begin(), 4);
///   // xs == {4, 3, 2, 1}
template <std::forward_iterator I, Integer N>
I reverse_n(I first, N n)
{
    if (n <= N{0}) {
        return first;
    }
    if (n == N{1}) {
        return ++first;
    }

    const N h = half(n);
    I middle = reverse_n(first, h);
    if (odd(n)) {
        ++middle;
    }
    I result = reverse_n(middle, h);
    (void)swap_ranges_n(first, middle, h);
    return result;
}

/// Reverse the first `n` elements of `[first, last)` (clipped to the range).
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4, 5};
///   mt::reverse_n(v.begin(), v.end(), 4);  // {4, 3, 2, 1, 5}
template <std::bidirectional_iterator I, std::sentinel_for<I> S, Integer N>
void reverse_n(I first, S last, N n)
{
    if (n <= N{0} || first == last) {
        return;
    }
    I mid = first;
    N k = n;
    while (k > N{0} && mid != last) {
        ++mid;
        --k;
    }
    std::reverse(first, mid);
}

template <std::forward_iterator I, std::sentinel_for<I> S, Integer N>
    requires(!std::bidirectional_iterator<I>)
I reverse_n(I first, S last, N n)
{
    return reverse_n(first, bounded_count(first, last, n));
}

template <std::forward_iterator I>
void reverse(I first, I last)
{
    reverse_n(first, static_cast<Difference_type<I>>(std::distance(first, last)));
}

template <std::ranges::forward_range R>
void reverse(R&& range)
{
    mt::reverse(std::ranges::begin(range), std::ranges::end(range));
}

/// Swap `[first, middle)` with `[middle, last)` from the outside in until one
/// side is exhausted. Returns the two unconsumed bounds.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4, 5};
///   mt::reverse_until(v.begin(), v.begin() + 2, v.end());
template <std::bidirectional_iterator I>
[[nodiscard]] constexpr std::pair<I, I> reverse_until(I first, I middle, I last)
{
    while (first != middle && middle != last) {
        --last;
        std::iter_swap(first, last);
        ++first;
    }
    return {first, last};
}

/// Reverse `n` forward-iterator elements using a bidirectional buffer.
///
/// Example:
///   std::list<int> xs{1, 2, 3, 4};
///   int buf[4];
///   mt::reverse_n_with_buffer(xs.begin(), 4, buf);  // xs == {4, 3, 2, 1}
template <std::forward_iterator I, Integer N, std::bidirectional_iterator B>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
I reverse_n_with_buffer(I first, N n, B buffer)
{
    auto [in, buffer_end] = mt::copy_n(first, n, buffer);
    (void)in;
    return std::reverse_copy(buffer, buffer_end, first);
}

template <std::forward_iterator I, std::sentinel_for<I> S, Integer N, std::bidirectional_iterator B>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
I reverse_n_with_buffer(I first, S last, N n, B buffer)
{
    auto [in, buffer_end] = mt::copy_n(first, last, n, buffer);
    (void)in;
    return std::reverse_copy(buffer, buffer_end, first);
}

/// Adaptive reverse: use buffer when `n_buf >= n`, otherwise fall back to
/// recursive `reverse_n` (EoP `reverse_n_adaptive` idea).
template <std::forward_iterator I, Integer N, std::bidirectional_iterator B>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
I reverse_n_adaptive(I first, N n, B buffer, N n_buf)
{
    if (n <= N{0}) {
        return first;
    }
    if (n_buf >= n) {
        return reverse_n_with_buffer(first, n, buffer);
    }
    return reverse_n(first, n);
}

/// Cycle values along a transformation of iterators (EoP `cycle_to`):
/// repeatedly exchange `*i` with `*f(i)` around the orbit.
template <std::forward_iterator I, typename F>
    requires std::invocable<F&, I> && std::same_as<std::invoke_result_t<F&, I>, I>
void cycle_to(I i, F f)
{
    I k = std::invoke(f, i);
    while (k != i) {
        std::iter_swap(i, k);
        k = std::invoke(f, k);
    }
}

/// Cycle values from a permutation: move `*f(i)` into `*i` along the orbit
/// (EoP `cycle_from`).
template <std::forward_iterator I, typename F>
    requires std::invocable<F&, I> && std::same_as<std::invoke_result_t<F&, I>, I>
void cycle_from(I i, F f)
{
    std::iter_value_t<I> tmp = std::move(*i);
    I j = i;
    I k = std::invoke(f, i);
    while (k != i) {
        *j = std::move(*k);
        j = k;
        k = std::invoke(f, k);
    }
    *j = std::move(tmp);
}

}  // namespace mt
