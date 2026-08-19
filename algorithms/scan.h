#pragma once

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace mt {

/// Inclusive prefix scan: `out[i] = op(in[0], ..., in[i])`.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4}, out(4);
///   mt::scan(v.begin(), v.end(), out.begin(), std::plus<>{});
///   // out == {1, 3, 6, 10}
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O,
          typename Op>
O scan(I first, S last, O dest, Op op)
{
    if (first == last) {
        return dest;
    }
    auto acc = *first;
    *dest = acc;
    ++dest;
    ++first;
    for (; first != last; ++first, ++dest) {
        acc = std::invoke(op, std::move(acc), *first);
        *dest = acc;
    }
    return dest;
}

template <std::ranges::input_range R, typename Op>
[[nodiscard]] auto scan(R&& range, Op op)
{
    std::vector<std::ranges::range_value_t<R>> out;
    scan(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out),
         std::move(op));
    return out;
}

/// Exclusive prefix scan: `out[0] = init`, `out[i] = op(init, in[0], ..., in[i-1])`.
///
/// Example:
///   std::vector<int> v{1, 2, 3}, out(3);
///   mt::exclusive_scan(v.begin(), v.end(), out.begin(), 0, std::plus<>{});
///   // out == {0, 1, 3}
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O,
          typename T, typename Op>
O exclusive_scan(I first, S last, O dest, T init, Op op)
{
    for (; first != last; ++first, ++dest) {
        *dest = init;
        init = std::invoke(op, std::move(init), *first);
    }
    return dest;
}

template <std::ranges::input_range R, typename T, typename Op>
[[nodiscard]] auto exclusive_scan(R&& range, T init, Op op)
{
    std::vector<T> out;
    exclusive_scan(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out),
                   std::move(init), std::move(op));
    return out;
}

}  // namespace mt
