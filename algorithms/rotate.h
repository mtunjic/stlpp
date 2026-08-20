#pragma once

#include "copy.h"
#include "concepts.h"
#include "iterator_ops.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Rotate `[first, last)` so `middle` becomes the new first element (EoP ch. 9).
/// Returns the iterator to the element that was previously at `first`
/// (`first + (last - middle)`). Used by `slide` / `shift_by` via `std::rotate`;
/// prefer `mt::rotate` when you need the EoP return value explicitly.
///
/// Example:
///   std::vector<int> v{0, 1, 2, 3, 4};
///   auto p = mt::rotate(v.begin(), v.begin() + 2, v.end());
///   // v == {2, 3, 4, 0, 1}, *p == 0
template <std::forward_iterator I>
[[nodiscard]] I rotate(I first, I middle, I last)
{
    if (middle == first) {
        return last;
    }
    if (middle == last) {
        return first;
    }
    return std::rotate(first, middle, last);
}

template <std::ranges::forward_range R>
[[nodiscard]] auto rotate(R& range, std::ranges::iterator_t<R> middle)
{
    return rotate(std::ranges::begin(range), middle, std::ranges::end(range));
}

/// Buffer-assisted rotate: copy `[first, middle)` aside, slide `[middle, last)`
/// left, then restore the prefix. Returns the same iterator as `rotate`.
template <std::forward_iterator I, std::forward_iterator B>
    requires std::indirectly_copyable<I, B> && std::indirectly_copyable<B, I>
[[nodiscard]] I rotate_with_buffer(I first, I middle, I last, B buffer)
{
    if (middle == first) {
        return last;
    }
    if (middle == last) {
        return first;
    }
    B buf_end = std::copy(first, middle, buffer);
    I mid_prime = std::copy(middle, last, first);
    std::copy(buffer, buf_end, mid_prime);
    return mid_prime;
}

/// Allocate a temporary buffer and rotate (convenience for forward iterators).
template <std::forward_iterator I>
[[nodiscard]] I rotate_with_buffer(I first, I middle, I last)
{
    if (middle == first) {
        return last;
    }
    if (middle == last) {
        return first;
    }
    using T = Value_type<I>;
    const auto n = static_cast<std::size_t>(std::distance(first, middle));
    std::vector<T> buf(n);
    return rotate_with_buffer(first, middle, last, buf.begin());
}

}  // namespace mt
