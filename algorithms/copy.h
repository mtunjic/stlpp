#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Copy `n` elements from `first` to `dest`. Returns `{in, out}` past the
/// last copied positions. `n == 0` is a no-op.
///
/// Example:
///   std::vector<int> src{1, 2, 3, 4};
///   std::vector<int> dst(2);
///   mt::copy_n(src.begin(), 2, dst.begin());  // dst == {1, 2}
template <std::input_iterator I, std::weakly_incrementable O, Integer N>
    requires std::indirectly_copyable<I, O>
[[nodiscard]] constexpr std::pair<I, O> copy_n(I first, N n, O dest)
{
    while (n != N{0}) {
        *dest = *first;
        ++first;
        ++dest;
        --n;
    }
    return {first, dest};
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
    auto [buf_again, out] = mt::copy_n(buffer, n, dest);
    (void)buf_end;
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
    while (n != N{0}) {
        std::iter_swap(first0, first1);
        ++first0;
        ++first1;
        --n;
    }
    return {first0, first1};
}

}  // namespace mt
