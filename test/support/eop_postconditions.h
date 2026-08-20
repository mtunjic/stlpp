#pragma once

#include "stlpp.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace mt::test {

template <typename Range1, typename Range2>
void expect_ranges_equal(const Range1& a, const Range2& b)
{
    ASSERT_EQ(std::distance(std::begin(a), std::end(a)), std::distance(std::begin(b), std::end(b)));
    EXPECT_TRUE(std::equal(std::begin(a), std::end(a), std::begin(b)));
}

template <std::forward_iterator I, typename P>
void expect_partitioned_at(I first, I mid, I last, P pred)
{
    EXPECT_TRUE(std::all_of(first, mid, pred));
    EXPECT_TRUE(std::none_of(mid, last, pred));
}

template <std::forward_iterator I, typename Comp = std::less<>>
void expect_sorted_prefix(I first, I last, Comp comp = {})
{
    EXPECT_TRUE(std::is_sorted(first, last, comp));
}

template <typename Range>
void expect_reverse_of(const Range& got, const Range& original)
{
    expect_ranges_equal(got, std::vector(std::rbegin(original), std::rend(original)));
}

template <typename Range, typename T, typename Op>
void expect_fold_matches_manual(const Range& range, T init, Op op)
{
    T manual = init;
    for (const auto& x : range) {
        manual = op(std::move(manual), x);
    }
    EXPECT_EQ(manual, mt::fold(range, init, op));
}

template <typename InputRange, typename OutputRange, typename Op>
void expect_inclusive_scan_prefix(const InputRange& input, const OutputRange& output, Op op)
{
    ASSERT_EQ(std::size(input), std::size(output));
    if (std::empty(input)) {
        return;
    }
    auto acc = *std::begin(input);
    EXPECT_EQ(*std::begin(output), acc);
    auto out_it = std::next(std::begin(output));
    for (auto in_it = std::next(std::begin(input)); in_it != std::end(input); ++in_it, ++out_it) {
        acc = op(std::move(acc), *in_it);
        EXPECT_EQ(*out_it, acc);
    }
}

template <typename InputRange, typename OutputRange, typename T, typename Op>
void expect_exclusive_scan_prefix(const InputRange& input, const OutputRange& output, T init, Op op)
{
    ASSERT_EQ(std::size(input), std::size(output));
    T acc = init;
    auto out_it = std::begin(output);
    for (const auto& x : input) {
        EXPECT_EQ(*out_it, acc);
        acc = op(std::move(acc), x);
        ++out_it;
    }
}

template <typename A, typename Op>
void expect_power_equals_repeat(A a, int n, Op op, A identity)
{
    if (n <= 0) {
        EXPECT_EQ(mt::power(a, n, op, identity), identity);
        return;
    }
    A repeated = a;
    for (int i = 1; i < n; ++i) {
        repeated = op(repeated, a);
    }
    EXPECT_EQ(mt::power(a, n, op), repeated);
}

template <typename N>
void expect_gcd_divides(N a, N b, N g)
{
    if (g == N{0}) {
        EXPECT_EQ(a, N{0});
        EXPECT_EQ(b, N{0});
        return;
    }
    const N ra = a % g;
    const N rb = b % g;
    EXPECT_EQ(ra, N{0}) << "g=" << g << " a=" << a;
    EXPECT_EQ(rb, N{0}) << "g=" << g << " b=" << b;
}

template <typename N>
void expect_extended_gcd_bezout(N a, N b)
{
    const auto [g, x, y] = mt::extended_gcd(a, b);
    expect_gcd_divides(a, b, g);
    EXPECT_EQ(a * x + b * y, g);
}

template <std::input_iterator I, typename P>
[[nodiscard]] std::vector<std::iter_value_t<I>> filter_copy(I first, I last, P pred)
{
    std::vector<std::iter_value_t<I>> out;
    for (; first != last; ++first) {
        if (std::invoke(pred, *first)) {
            out.push_back(*first);
        }
    }
    return out;
}

template <std::forward_iterator I, typename P>
void expect_remove_if_not_law(I first, I last, P pred)
{
    const auto expected = filter_copy(first, last, pred);
    const auto p = mt::remove_if_not(first, last, pred);
    expect_ranges_equal(std::vector<std::iter_value_t<I>>(first, p), expected);
}

template <std::bidirectional_iterator I, typename P>
void expect_remove_if_not_backward_law(I first, I last, P pred)
{
    const auto expected = filter_copy(first, last, pred);
    const auto w = mt::remove_if_not_backward(first, last, pred);
    expect_ranges_equal(std::vector<std::iter_value_t<I>>(w, last), expected);
}

template <std::input_iterator I, typename P>
void expect_span_if_law(I first, I last, P pred)
{
    const auto mid = mt::span_if(first, last, pred);
    EXPECT_TRUE(std::all_of(first, mid, pred));
    if (mid != last) {
        EXPECT_FALSE(std::invoke(pred, *mid));
    }
}

template <std::input_iterator I, typename P>
void expect_break_if_law(I first, I last, P pred)
{
    const auto mid = mt::break_if(first, last, pred);
    EXPECT_TRUE(std::none_of(first, mid, pred));
    if (mid != last) {
        EXPECT_TRUE(std::invoke(pred, *mid));
    }
}

template <std::input_iterator I, Integer N>
void expect_split_at_law(I first, I last, N n)
{
    const auto mid = mt::split_at(first, last, n);
    const auto dist = std::distance(first, mid);
    const auto max_dist = std::distance(first, last);
    EXPECT_EQ(dist, std::min<N>(n, static_cast<N>(max_dist)));
}

template <typename InputRange, typename F>
void expect_mapcat_manual(const InputRange& input, F f)
{
    std::vector<std::ranges::range_value_t<
        std::invoke_result_t<F, std::ranges::range_value_t<InputRange>>>>
        manual;
    for (const auto& x : input) {
        const auto sub = std::invoke(f, x);
        manual.insert(manual.end(), std::begin(sub), std::end(sub));
    }
    expect_ranges_equal(mt::mapcat(input, f), manual);
}

template <std::forward_iterator I, std::sentinel_for<I> S, typename F>
void expect_apply_maps(I first, S last, F f)
{
    using V = std::iter_value_t<I>;
    const std::vector<V> before(first, last);
    std::vector<V> expected;
    expected.reserve(before.size());
    for (const V& x : before) {
        expected.push_back(std::invoke(f, x));
    }
    mt::apply(first, last, f);
    expect_ranges_equal(std::vector<V>(first, last), expected);
}

template <std::forward_iterator I, std::sentinel_for<I> S, typename F, typename P>
void expect_apply_until_law(I first, S last, F f, P stop_pred)
{
    using V = std::iter_value_t<I>;
    std::vector<V> expected(first, last);
    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (std::invoke(stop_pred, expected[i])) {
            break;
        }
        expected[i] = std::invoke(f, expected[i]);
    }
    const auto stop = mt::apply_until(first, last, f, stop_pred);
    expect_ranges_equal(std::vector<V>(first, last), expected);
    if (stop != last) {
        EXPECT_TRUE(std::invoke(stop_pred, *stop));
    }
}

template <typename Range, typename F>
void expect_apply_maps(Range& range, F f)
{
    expect_apply_maps(std::begin(range), std::end(range), f);
}

template <typename Range, typename F, typename P>
void expect_apply_until_law(Range& range, F f, P stop_pred)
{
    expect_apply_until_law(std::begin(range), std::end(range), f, stop_pred);
}

template <typename Range, typename P>
void expect_quantifiers_match_std(const Range& range, P pred)
{
    EXPECT_EQ(mt::all(range, pred),
              std::all_of(std::begin(range), std::end(range), pred));
    EXPECT_EQ(mt::any(range, pred),
              std::any_of(std::begin(range), std::end(range), pred));
    EXPECT_EQ(mt::none(range, pred),
              std::none_of(std::begin(range), std::end(range), pred));
    EXPECT_EQ(mt::count_if(range, pred),
              std::count_if(std::begin(range), std::end(range), pred));
}

template <std::random_access_iterator I>
void expect_slice_law(I first, I last, std::iter_difference_t<I> start,
                      std::iter_difference_t<I> count)
{
    const auto [a, b] = mt::slice(first, last, start, count);
    const auto n = std::distance(first, last);
    if (start < 0) {
        start = 0;
    }
    if (start > n) {
        start = n;
    }
    if (count < 0) {
        count = 0;
    }
    const auto remaining = n - start;
    if (count > remaining) {
        count = remaining;
    }
    EXPECT_EQ(std::distance(a, b), count);
    if (count > 0) {
        EXPECT_EQ(*a, *(first + start));
    }
}

template <std::bidirectional_iterator I, typename P>
void expect_gather_law(I first, I last, I pos, P pred)
{
    const auto [lo, hi] = mt::gather(first, last, pos, pred);
    EXPECT_TRUE(std::all_of(lo, hi, pred));
    const auto kept = std::distance(lo, hi);
    const auto expected = mt::count_if(first, last, pred);
    EXPECT_EQ(kept, expected);
}

template <typename T>
void expect_transpose_roundtrip(const std::vector<std::vector<T>>& matrix)
{
    expect_ranges_equal(mt::transpose(mt::transpose(matrix)), matrix);
}

template <std::forward_iterator I, typename V>
void expect_iota_law(I first, I last, V start)
{
    mt::iota(first, last, start);
    V value = start;
    for (I it = first; it != last; ++it, ++value) {
        EXPECT_EQ(*it, value);
    }
}

template <std::forward_iterator I>
void expect_reverse_iota_law(I first, I last)
{
    mt::reverse_iota(first, last);
    const auto n = std::distance(first, last);
    auto expected = static_cast<std::iter_value_t<I>>(n);
    for (I it = first; it != last; ++it) {
        --expected;
        EXPECT_EQ(*it, expected);
    }
}

template <typename Range1, typename Range2>
void expect_zip_unzip_roundtrip(const Range1& a, const Range2& b)
{
    const auto zipped = mt::zip(a, b);
    using T1 = std::ranges::range_value_t<Range1>;
    using T2 = std::ranges::range_value_t<Range2>;
    std::vector<T1> out_a(zipped.size());
    std::vector<T2> out_b(zipped.size());
    (void)mt::unzip(zipped.begin(), zipped.end(), out_a.begin(), out_b.begin());
    expect_ranges_equal(out_a, a);
    expect_ranges_equal(out_b, b);
}

template <std::forward_iterator I>
void expect_min_max_law(I first, I last)
{
    const auto [lo, hi] = mt::min_max(first, last);
    if (first == last) {
        EXPECT_EQ(lo, first);
        EXPECT_EQ(hi, first);
        return;
    }
    for (I it = first; it != last; ++it) {
        EXPECT_LE(*lo, *it);
        EXPECT_LE(*it, *hi);
    }
}

template <typename Range1, typename Range2>
void expect_merge_sorted_union(const Range1& a, const Range2& b)
{
    const auto merged = mt::merge(a, b);
    expect_sorted_prefix(merged.begin(), merged.end());
    EXPECT_EQ(merged.size(), std::size(a) + std::size(b));
}

template <typename Range1, typename Range2>
void expect_set_union_sorted(const Range1& a, const Range2& b)
{
    const auto out = mt::set_union(a, b);
    expect_sorted_prefix(out.begin(), out.end());
    EXPECT_EQ(out, (std::vector<std::ranges::range_value_t<Range1>>{1, 2, 3}));
}

template <typename Range1, typename Range2>
void expect_set_intersection_sorted(const Range1& a, const Range2& b)
{
    const auto out = mt::set_intersection(a, b);
    expect_sorted_prefix(out.begin(), out.end());
    EXPECT_EQ(out, (std::vector<std::ranges::range_value_t<Range1>>{2, 3}));
}

template <typename Range, typename T>
void expect_border_1d(const Range& input, const T& fill)
{
    const auto out = mt::border(input, fill);
    ASSERT_EQ(out.size(), std::size(input) + 2);
    EXPECT_EQ(out.front(), fill);
    EXPECT_EQ(out.back(), fill);
    expect_ranges_equal(std::vector(out.begin() + 1, out.end() - 1), input);
}

template <typename BidirectionalRange>
void expect_combination_walk(BidirectionalRange& v, std::size_t k, std::size_t expected_count)
{
    if (v.empty() || k == 0 || k > v.size()) {
        return;
    }
    const auto mid = std::next(v.begin(), static_cast<std::ptrdiff_t>(k));
    std::size_t count = 0;
    do {
        ++count;
        EXPECT_TRUE(std::is_sorted(v.begin(), mid));
        EXPECT_TRUE(std::is_sorted(mid, v.end()));
    } while (mt::next_combination(v.begin(), mid, v.end()));
    EXPECT_EQ(count, expected_count);
}

inline void expect_matches_at_valid_offsets(std::string_view text, std::string_view pattern,
                                            const std::vector<std::size_t>& expected)
{
    const auto hits = mt::matches(text, pattern);
    EXPECT_EQ(hits, expected);
    for (std::size_t i : hits) {
        ASSERT_LE(i + pattern.size(), text.size());
        EXPECT_EQ(text.substr(i, pattern.size()), pattern);
    }
}

template <typename Range>
void expect_random_iota_permutation(Range& range)
{
    const auto n = static_cast<std::size_t>(std::distance(std::begin(range), std::end(range)));
    if (n == 0) {
        return;
    }
    std::vector<std::ranges::range_value_t<Range>> sorted(std::begin(range), std::end(range));
    std::sort(sorted.begin(), sorted.end());
    std::vector<std::ranges::range_value_t<Range>> expected(n);
    std::iota(expected.begin(), expected.end(), 0);
    expect_ranges_equal(sorted, expected);
}

template <typename Range, typename T>
void expect_random_in_closed_bounds(Range& range, T lo, T hi)
{
    const auto [a, b] = std::minmax(lo, hi);
    for (const auto& x : range) {
        EXPECT_GE(x, a);
        EXPECT_LE(x, b);
    }
}

}  // namespace mt::test
