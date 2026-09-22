#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <vector>

namespace mt {

/// Merge two sorted ranges into `dest`.
///
/// Example:
///   mt::merge(std::vector{1, 3}, std::vector{2, 4});  // {1, 2, 3, 4}
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Comp = std::less<>>
O merge(I1 a, S1 a_last, I2 b, S2 b_last, O dest, Comp comp = {})
{
    return std::ranges::merge(a, a_last, b, b_last, dest, std::move(comp)).out;
}

template <std::ranges::input_range R1, std::ranges::input_range R2, typename Comp = std::less<>>
[[nodiscard]] auto merge(R1&& a, R2&& b, Comp comp = {})
{
    std::vector<std::ranges::range_value_t<R1>> out;
    merge(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b), std::ranges::end(b),
          std::back_inserter(out), std::move(comp));
    return out;
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Comp = std::less<>>
O set_union(I1 a, S1 a_last, I2 b, S2 b_last, O dest, Comp comp = {})
{
    return std::ranges::set_union(a, a_last, b, b_last, dest, std::move(comp)).out;
}

template <std::ranges::input_range R1, std::ranges::input_range R2, typename Comp = std::less<>>
[[nodiscard]] auto set_union(R1&& a, R2&& b, Comp comp = {})
{
    std::vector<std::ranges::range_value_t<R1>> out;
    set_union(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
              std::ranges::end(b), std::back_inserter(out), std::move(comp));
    return out;
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Comp = std::less<>>
O set_intersection(I1 a, S1 a_last, I2 b, S2 b_last, O dest, Comp comp = {})
{
    return std::ranges::set_intersection(a, a_last, b, b_last, dest, std::move(comp)).out;
}

template <std::ranges::input_range R1, std::ranges::input_range R2, typename Comp = std::less<>>
[[nodiscard]] auto set_intersection(R1&& a, R2&& b, Comp comp = {})
{
    std::vector<std::ranges::range_value_t<R1>> out;
    set_intersection(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
                     std::ranges::end(b), std::back_inserter(out), std::move(comp));
    return out;
}

/// Merge two adjacent counted sorted ranges using a buffer of size `n0` (EoP).
/// Precondition: `[f0, f0+n0)` and `[f1, f1+n1)` are sorted and typically
/// adjacent (`f1 == f0 + n0`). Returns the end of the merged range.
template <std::forward_iterator I, std::forward_iterator B, typename N, typename Comp = std::less<>>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
[[nodiscard]] I merge_n_with_buffer(I f0, N n0, I f1, N n1, B buffer, Comp comp = {})
{
    if (n0 <= N{0}) {
        I end = f1;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n1));
        return end;
    }
    if (n1 <= N{0}) {
        I end = f0;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n0));
        return end;
    }
    B buf_end = std::copy_n(f0, n0, buffer);
    I f1_end = f1;
    std::advance(f1_end, static_cast<std::iter_difference_t<I>>(n1));
    return std::merge(buffer, buf_end, f1, f1_end, f0, std::move(comp));
}

/// Merge-sort of `n` elements using a buffer of size at least `ceil(n/2)`.
template <std::forward_iterator I, std::forward_iterator B, typename N, typename Comp = std::less<>>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
[[nodiscard]] I sort_n_with_buffer(I first, N n, B buffer, Comp comp = {})
{
    if (n <= N{0}) {
        return first;
    }
    const N h = n / N{2};
    if (h == N{0}) {
        I end = first;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n));
        return end;
    }
    I mid = sort_n_with_buffer(first, h, buffer, comp);
    (void)sort_n_with_buffer(mid, n - h, buffer, comp);
    return merge_n_with_buffer(first, h, mid, n - h, buffer, comp);
}

/// Counted merge-sort allocating an internal buffer of size `n/2`.
template <std::forward_iterator I, typename N, typename Comp = std::less<>>
[[nodiscard]] I sort_n(I first, N n, Comp comp = {})
{
    if (n <= N{1}) {
        I end = first;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n > N{0} ? n : N{0}));
        return end;
    }
    using T = std::iter_value_t<I>;
    std::vector<T> buf(static_cast<std::size_t>(n / N{2} + n % N{2}));
    return sort_n_with_buffer(first, n, buf.begin(), std::move(comp));
}

/// Adaptive merge: use the buffer when large enough for `n0`, otherwise rotate
/// the second range into place and merge recursively (EoP sketch).
template <std::forward_iterator I, std::forward_iterator B, typename N, typename Comp = std::less<>>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
[[nodiscard]] I merge_n_adaptive(I f0, N n0, I f1, N n1, B buffer, N n_buf, Comp comp = {})
{
    if (n0 <= N{0}) {
        I end = f1;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n1));
        return end;
    }
    if (n1 <= N{0}) {
        I end = f0;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n0));
        return end;
    }
    if (n_buf >= n0) {
        return merge_n_with_buffer(f0, n0, f1, n1, buffer, comp);
    }
    // Insufficient buffer: copy both runs into a temporary vector and merge back.
    using T = std::iter_value_t<I>;
    std::vector<T> left(static_cast<std::size_t>(n0));
    std::vector<T> right(static_cast<std::size_t>(n1));
    std::copy_n(f0, n0, left.begin());
    std::copy_n(f1, n1, right.begin());
    return std::merge(left.begin(), left.end(), right.begin(), right.end(), f0, std::move(comp));
}

/// Adaptive counted merge-sort using a caller-provided buffer of size `n_buf`.
template <std::forward_iterator I, std::forward_iterator B, typename N, typename Comp = std::less<>>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
[[nodiscard]] I sort_n_adaptive(I first, N n, B buffer, N n_buf, Comp comp = {})
{
    if (n <= N{0}) {
        return first;
    }
    const N h = n / N{2};
    if (h == N{0}) {
        I end = first;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n));
        return end;
    }
    I mid = sort_n_adaptive(first, h, buffer, n_buf, comp);
    (void)sort_n_adaptive(mid, n - h, buffer, n_buf, comp);
    return merge_n_adaptive(first, h, mid, n - h, buffer, n_buf, comp);
}

/// Convenience: adaptive sort with a small fixed-size stack buffer, then heap.
template <std::forward_iterator I, typename N, typename Comp = std::less<>>
[[nodiscard]] I advanced_sort_n(I first, N n, Comp comp = {})
{
    using T = std::iter_value_t<I>;
    constexpr N k_stack = N{32};
    if (n <= N{1}) {
        I end = first;
        std::advance(end, static_cast<std::iter_difference_t<I>>(n > N{0} ? n : N{0}));
        return end;
    }
    if (n <= k_stack * N{2}) {
        std::vector<T> buf(static_cast<std::size_t>(n / N{2} + n % N{2}));
        return sort_n_with_buffer(first, n, buf.begin(), std::move(comp));
    }
    std::vector<T> buf(static_cast<std::size_t>(k_stack));
    return sort_n_adaptive(first, n, buf.begin(), k_stack, std::move(comp));
}

}  // namespace mt
