#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

namespace mt {

/// Map with carried state. `f(acc, x) -> {new_acc, y}`.
///
/// Example:
///   auto f = [](int acc, int x) { return std::pair{acc + x, acc + x}; };
///   auto [sum, prefixes] = mt::map_accum(std::vector{1, 2, 3}, 0, f);
///   // sum == 6, prefixes == {1, 3, 6}
template <std::input_iterator I, std::sentinel_for<I> S, typename T, typename F,
          std::weakly_incrementable O>
std::pair<T, O> map_accum(I first, S last, T acc, F f, O dest)
{
    for (; first != last; ++first, ++dest) {
        auto [next, y] = std::invoke(f, std::move(acc), *first);
        acc = std::move(next);
        *dest = std::move(y);
    }
    return {std::move(acc), dest};
}

template <std::ranges::input_range R, typename T, typename F>
[[nodiscard]] auto map_accum(R&& range, T acc, F f)
{
    using Y =
        std::tuple_element_t<1, std::invoke_result_t<F&, T, std::ranges::range_reference_t<R>>>;
    std::vector<std::remove_cvref_t<Y>> out;
    auto result = map_accum(std::ranges::begin(range), std::ranges::end(range), std::move(acc),
                            std::move(f), std::back_inserter(out));
    return std::pair{std::move(result.first), std::move(out)};
}

/// Consecutive runs where `same(a, b)` holds (Haskell `groupBy`).
///
/// Example:
///   mt::group_by(std::vector{1, 1, 2, 2, 1}, std::equal_to<>{});
template <std::input_iterator I, std::sentinel_for<I> S, typename Pred, std::weakly_incrementable O>
O group_by(I first, S last, Pred same, O dest)
{
    while (first != last) {
        std::vector<std::iter_value_t<I>> run;
        auto head = *first;
        run.push_back(head);
        ++first;
        while (first != last && std::invoke(same, head, *first)) {
            run.push_back(*first);
            ++first;
        }
        *dest = std::move(run);
        ++dest;
    }
    return dest;
}

template <std::ranges::input_range R, typename Pred>
[[nodiscard]] auto group_by(R&& range, Pred same)
{
    std::vector<std::vector<std::ranges::range_value_t<R>>> out;
    group_by(std::ranges::begin(range), std::ranges::end(range), std::move(same),
             std::back_inserter(out));
    return out;
}

/// Dot product (default `+` of `*`). Stops when either range ends.
///
/// Example:
///   mt::inner_product(std::vector{1, 2, 3}, std::vector{4, 5, 6});  // 32
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, typename T, typename Op1 = std::plus<>,
          typename Op2 = std::multiplies<>>
    requires(!std::convertible_to<S2, T>)
[[nodiscard]] T inner_product(I1 a, S1 a_last, I2 b, S2 b_last, T init, Op1 add = {}, Op2 mul = {})
{
    for (; a != a_last && b != b_last; ++a, ++b) {
        init = std::invoke(add, std::move(init), std::invoke(mul, *a, *b));
    }
    return init;
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
[[nodiscard]] auto inner_product(R1&& a, R2&& b)
{
    using T = std::ranges::range_value_t<R1>;
    return inner_product(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
                         std::ranges::end(b), T{});
}

/// `out[0] = in[0]`; `out[i] = op(in[i], in[i-1])` (default minus).
///
/// Example:
///   mt::adjacent_difference(std::vector{1, 3, 6, 10});  // {1, 2, 3, 4}
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O,
          typename Op = std::minus<>>
O adjacent_difference(I first, S last, O dest, Op op = {})
{
    return std::adjacent_difference(first, last, dest, std::move(op));
}

template <std::ranges::input_range R, typename Op = std::minus<>>
[[nodiscard]] auto adjacent_difference(R&& range, Op op = {})
{
    std::vector<std::ranges::range_value_t<R>> out;
    adjacent_difference(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(out),
                        std::move(op));
    return out;
}

/// Split into two output ranges by predicate.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4}, evens, odds;
///   mt::partition_copy(v, std::back_inserter(evens), std::back_inserter(odds),
///                      [](int x) { return x % 2 == 0; });
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O1,
          std::weakly_incrementable O2, typename P>
std::pair<O1, O2> partition_copy(I first, S last, O1 yes, O2 no, P pred)
{
    auto [y, n] = std::partition_copy(first, last, yes, no, std::move(pred));
    return {y, n};
}

template <std::ranges::input_range R, typename P>
[[nodiscard]] auto partition_copy(R&& range, P pred)
{
    using T = std::ranges::range_value_t<R>;
    std::vector<T> yes;
    std::vector<T> no;
    partition_copy(std::ranges::begin(range), std::ranges::end(range), std::back_inserter(yes),
                   std::back_inserter(no), std::move(pred));
    return std::pair{std::move(yes), std::move(no)};
}

/// Cartesian product of two ranges as pairs.
///
/// Example:
///   mt::cartesian_product(std::vector{1, 2}, std::vector{'a', 'b'});
///   // {(1,'a'), (1,'b'), (2,'a'), (2,'b')}
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::forward_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O>
O cartesian_product(I1 a, S1 a_last, I2 b, S2 b_last, O dest)
{
    std::vector<std::iter_value_t<I2>> b_copy(b, b_last);
    for (; a != a_last; ++a) {
        for (const auto& y : b_copy) {
            *dest = std::pair<std::iter_value_t<I1>, std::iter_value_t<I2>>{*a, y};
            ++dest;
        }
    }
    return dest;
}

template <std::ranges::input_range R1, std::ranges::forward_range R2>
[[nodiscard]] auto cartesian_product(R1&& a, R2&& b)
{
    using P = std::pair<std::ranges::range_value_t<R1>, std::ranges::range_value_t<R2>>;
    std::vector<P> out;
    cartesian_product(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
                      std::ranges::end(b), std::back_inserter(out));
    return out;
}

/// In-place next permutation. Returns false when wrapping to sorted order.
template <std::bidirectional_iterator I, typename Comp = std::less<>>
bool next_permutation(I first, I last, Comp comp = {})
{
    return std::next_permutation(first, last, std::move(comp));
}

template <std::ranges::bidirectional_range R, typename Comp = std::less<>>
bool next_permutation(R&& range, Comp comp = {})
{
    return next_permutation(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
}

}  // namespace mt
