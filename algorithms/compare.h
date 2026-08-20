#pragma once

#include "concepts.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

namespace detail {

template <typename R>
struct complement_of_converse {
    R r;
    template <typename T, typename U>
    constexpr bool operator()(const T& x, const U& y) const
    {
        return !std::invoke(r, y, x);
    }
};

}  // namespace detail

/// First pair of positions where `r(*f0, *f1)` is false, or both range ends.
///
/// Example:
///   std::vector<int> a{1, 2, 3}, b{1, 2, 9};
///   auto [i, j] = mt::find_mismatch(a.begin(), a.end(), b.begin(), b.end(),
///                                   std::equal_to<>{});  // *i == 3, *j == 9
template <std::input_iterator I0, std::input_iterator I1, typename R>
    requires std::predicate<R, std::iter_reference_t<I0>, std::iter_reference_t<I1>>
[[nodiscard]] constexpr std::pair<I0, I1> find_mismatch(I0 f0, I0 l0, I1 f1, I1 l1, R r)
{
    while (f0 != l0 && f1 != l1 && std::invoke(r, *f0, *f1)) {
        ++f0;
        ++f1;
    }
    return {f0, f1};
}

template <std::input_iterator I0, std::input_iterator I1>
[[nodiscard]] constexpr std::pair<I0, I1> find_mismatch(I0 f0, I0 l0, I1 f1, I1 l1)
{
    return find_mismatch(f0, l0, f1, l1, std::equal_to<>{});
}

template <std::ranges::input_range R0, std::ranges::input_range R1, typename Rel>
[[nodiscard]] constexpr auto find_mismatch(R0&& r0, R1&& r1, Rel r)
{
    return find_mismatch(std::ranges::begin(r0), std::ranges::end(r0),
                         std::ranges::begin(r1), std::ranges::end(r1), std::move(r));
}

template <std::ranges::input_range R0, std::ranges::input_range R1>
[[nodiscard]] constexpr auto find_mismatch(R0&& r0, R1&& r1)
{
    return find_mismatch(std::forward<R0>(r0), std::forward<R1>(r1), std::equal_to<>{});
}

/// First adjacent pair `(prev, cur)` where `r(*prev, *cur)` is false.
template <std::input_iterator I, typename R>
    requires std::predicate<R, std::iter_reference_t<I>, std::iter_reference_t<I>>
[[nodiscard]] constexpr I find_adjacent_mismatch(I f, I l, R r)
{
    if (f == l) {
        return l;
    }
    auto x = *f;
    ++f;
    while (f != l && std::invoke(r, x, *f)) {
        x = *f;
        ++f;
    }
    return f;
}

/// Forward-iterator variant (EoP): returns the second iterator of the first mismatching pair.
template <std::forward_iterator I, typename R>
    requires std::predicate<R, std::iter_reference_t<I>, std::iter_reference_t<I>>
[[nodiscard]] constexpr I find_adjacent_mismatch_forward(I f, I l, R r)
{
    if (f == l) {
        return l;
    }
    I t;
    do {
        t = f;
        ++f;
    } while (f != l && std::invoke(r, *t, *f));
    return f;
}

/// True when relation `r` holds between every adjacent pair.
template <std::input_iterator I, typename R>
    requires std::predicate<R, std::iter_reference_t<I>, std::iter_reference_t<I>>
[[nodiscard]] constexpr bool relation_preserving(I f, I l, R r)
{
    return l == find_adjacent_mismatch(f, l, r);
}

/// Strict weak-ordering chain: `r(*a, *b)` for every adjacent `(a, b)`.
template <std::input_iterator I, typename R = std::less<>>
    requires std::predicate<R, std::iter_reference_t<I>, std::iter_reference_t<I>>
[[nodiscard]] constexpr bool strictly_increasing_range(I f, I l, R r = {})
{
    return relation_preserving(f, l, r);
}

/// Non-decreasing chain: `!r(*b, *a)` for every adjacent `(a, b)`.
template <std::input_iterator I, typename R = std::less<>>
    requires std::predicate<R, std::iter_reference_t<I>, std::iter_reference_t<I>>
[[nodiscard]] constexpr bool increasing_range(I f, I l, R r = {})
{
    return relation_preserving(f, l, detail::complement_of_converse<R>{r});
}

/// True when `pred` is false on a prefix and true on the suffix (EoP ch. 6).
/// Equivalently: `last == find_if_not(find_if(first, last, pred), last, pred)`.
template <std::input_iterator I, typename P>
    requires std::predicate<P, std::iter_reference_t<I>>
[[nodiscard]] constexpr bool partitioned(I f, I l, P p)
{
    return l == std::find_if_not(std::find_if(f, l, p), l, p);
}

/// Both ranges equivalent element-wise under equivalence relation `r`.
template <std::input_iterator I0, std::input_iterator I1, typename R>
    requires std::predicate<R, std::iter_reference_t<I0>, std::iter_reference_t<I1>>
[[nodiscard]] constexpr bool lexicographical_equivalent(I0 f0, I0 l0, I1 f1, I1 l1, R r)
{
    const auto [m0, m1] = find_mismatch(f0, l0, f1, l1, r);
    return m0 == l0 && m1 == l1;
}

template <std::input_iterator I0, std::input_iterator I1>
[[nodiscard]] constexpr bool lexicographical_equal(I0 f0, I0 l0, I1 f1, I1 l1)
{
    return lexicographical_equivalent(f0, l0, f1, l1, std::equal_to<>{});
}

/// True when the first range is lexicographically less than the second under `r`.
template <std::input_iterator I0, std::input_iterator I1, typename R>
    requires std::predicate<R, std::iter_reference_t<I0>, std::iter_reference_t<I1>>
[[nodiscard]] constexpr bool lexicographical_compare(I0 f0, I0 l0, I1 f1, I1 l1, R r)
{
    while (true) {
        if (f1 == l1) {
            return false;
        }
        if (f0 == l0) {
            return true;
        }
        if (std::invoke(r, *f0, *f1)) {
            return true;
        }
        if (std::invoke(r, *f1, *f0)) {
            return false;
        }
        ++f0;
        ++f1;
    }
}

template <std::input_iterator I0, std::input_iterator I1>
[[nodiscard]] constexpr bool lexicographical_less(I0 f0, I0 l0, I1 f1, I1 l1)
{
    return lexicographical_compare(f0, l0, f1, l1, std::less<>{});
}

template <std::ranges::input_range R0, std::ranges::input_range R1, typename Rel>
[[nodiscard]] constexpr bool lexicographical_equivalent(R0&& r0, R1&& r1, Rel rel)
{
    return lexicographical_equivalent(std::ranges::begin(r0), std::ranges::end(r0),
                                      std::ranges::begin(r1), std::ranges::end(r1),
                                      std::move(rel));
}

template <std::ranges::input_range R0, std::ranges::input_range R1>
[[nodiscard]] constexpr bool lexicographical_equal(R0&& r0, R1&& r1)
{
    return lexicographical_equal(std::ranges::begin(r0), std::ranges::end(r0),
                                 std::ranges::begin(r1), std::ranges::end(r1));
}

template <std::ranges::input_range R0, std::ranges::input_range R1, typename Rel>
[[nodiscard]] constexpr bool lexicographical_compare(R0&& r0, R1&& r1, Rel rel)
{
    return lexicographical_compare(std::ranges::begin(r0), std::ranges::end(r0),
                                   std::ranges::begin(r1), std::ranges::end(r1),
                                   std::move(rel));
}

template <std::ranges::input_range R0, std::ranges::input_range R1>
[[nodiscard]] constexpr bool lexicographical_less(R0&& r0, R1&& r1)
{
    return lexicographical_less(std::ranges::begin(r0), std::ranges::end(r0),
                                std::ranges::begin(r1), std::ranges::end(r1));
}

}  // namespace mt
