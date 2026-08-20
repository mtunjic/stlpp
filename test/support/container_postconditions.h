#pragma once

#include "eop_postconditions.h"

#include "stlpp.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace mt::test {

template <typename InputRange, typename OutputRange, typename F>
void expect_map_elementwise(const InputRange& input, const OutputRange& output, F f)
{
    ASSERT_EQ(std::distance(std::begin(input), std::end(input)),
              std::distance(std::begin(output), std::end(output)));
    auto out_it = std::begin(output);
    for (const auto& x : input) {
        EXPECT_EQ(*out_it, std::invoke(f, x));
        ++out_it;
    }
}

template <typename Range, typename Comp = std::less<>>
void expect_sorted_like_std(Range& range, Comp comp = {})
{
    auto expected = std::vector<typename Range::value_type>(range.begin(), range.end());
    std::sort(expected.begin(), expected.end(), comp);
    expect_ranges_equal(range, expected);
}

template <typename Range, typename P>
void expect_pfind_leftmost(Range& range, P pred)
{
    const auto par = mt::pfind_if(range, pred);
    const auto seq = std::find_if(range.begin(), range.end(), pred);
    EXPECT_EQ(par, seq);
}

template <typename Range, typename T>
void expect_pfind_value_leftmost(Range& range, const T& value)
{
    expect_pfind_leftmost(range, [&](const auto& x) { return x == value; });
}

template <typename Range, typename T, typename Op>
void expect_parallel_reduce_matches_fold(const Range& range, T init, Op op)
{
    expect_fold_matches_manual(range, init, op);
    EXPECT_EQ(mt::reduce(range, init, op), mt::fold(range, init, op));
}

template <typename T>
void expect_tape_invariants(const mt::tape<T>& t)
{
    EXPECT_GE(t.size(), 1u);
    EXPECT_LT(t.position(), t.size());
}

template <typename K, typename V>
void expect_htable_roundtrip(mt::htable<K, V>& t, K key, const V& value)
{
    EXPECT_TRUE(t.insert(key, value));
    ASSERT_NE(t.find(key), nullptr);
    EXPECT_EQ(*t.find(key), value);
    EXPECT_FALSE(t.insert(key, V{}));
}

template <typename Compare>
void expect_pqueue_pop_order(mt::dynamic_pqueue<int, Compare>& q, const std::vector<int>& expected)
{
    for (int x : expected) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.top(), x);
        q.pop();
    }
    EXPECT_TRUE(q.empty());
}

inline void expect_quadtree_preserves_inserted(const mt::quadtree& q, const mt::rect_2d& bounds,
                                               std::size_t count)
{
    EXPECT_EQ(q.query(bounds).size(), count);
}

template <typename Stack>
void expect_ts_stack_lifo(Stack& s, const std::vector<int>& push_order)
{
    for (int x : push_order) {
        s.push(x);
    }
    for (auto it = push_order.rbegin(); it != push_order.rend(); ++it) {
        auto v = s.try_pop();
        ASSERT_TRUE(v.has_value());
        EXPECT_EQ(*v, *it);
    }
    EXPECT_TRUE(s.empty());
}

template <typename Queue>
void expect_ts_queue_fifo(Queue& q, const std::vector<int>& push_order)
{
    for (int x : push_order) {
        q.push(x);
    }
    for (int x : push_order) {
        auto v = q.try_pop();
        ASSERT_TRUE(v.has_value());
        EXPECT_EQ(*v, x);
    }
    EXPECT_TRUE(q.empty());
}

template <typename PQueue>
void expect_ts_priority_pop_order(PQueue& pq, const std::vector<int>& push_values,
                                  const std::vector<int>& pop_order)
{
    for (int x : push_values) {
        pq.push(x);
    }
    for (int x : pop_order) {
        auto v = pq.try_pop();
        ASSERT_TRUE(v.has_value());
        EXPECT_EQ(*v, x);
    }
    EXPECT_TRUE(pq.empty());
}

}  // namespace mt::test
