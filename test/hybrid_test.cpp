#include "stlpp.h"

#include <algorithm>
#include <functional>
#include <list>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

std::vector<int> iota_n(int n)
{
    std::vector<int> v(static_cast<std::size_t>(n));
    std::iota(v.begin(), v.end(), 0);
    return v;
}

}  // namespace

TEST(Map, Range_Squares) {
    auto out = mt::map(std::vector{1, 2, 3}, [](int x) { return x * x; });
    EXPECT_EQ(out, (std::vector<int>{1, 4, 9}));
}

TEST(Map, DestAndForwardIterator) {
    std::vector<int> v{1, 2, 3};
    std::vector<int> out(3);
    mt::map(v.begin(), v.end(), out.begin(), [](int x) { return x + 1; });
    EXPECT_EQ(out, (std::vector<int>{2, 3, 4}));

    std::list<int> lst{1, 2, 3};
    std::vector<int> from_list;
    mt::map(lst.begin(), lst.end(), std::back_inserter(from_list), [](int x) { return x * 2; });
    EXPECT_EQ(from_list, (std::vector<int>{2, 4, 6}));
}

TEST(Map, EmptyAndAboveCutoff) {
    EXPECT_TRUE(mt::map(std::vector<int>{}, [](int x) { return x; }).empty());

    auto v = iota_n(static_cast<int>(mt::par::default_cutoff) + 100);
    auto out = mt::map(v, [](int x) { return x * 2; });
    ASSERT_EQ(out.size(), v.size());
    EXPECT_EQ(out.front(), 0);
    EXPECT_EQ(out.back(), v.back() * 2);
}

TEST(Fold, LeftFoldPlus) {
    std::vector<int> v{1, 2, 3, 4};
    EXPECT_EQ(mt::fold(v, 0, std::plus<>{}), 10);
    EXPECT_EQ(mt::fold(std::vector<int>{}, 7, std::plus<>{}), 7);
}

TEST(Reduce, MatchesFoldOnPlus) {
    auto v = iota_n(static_cast<int>(mt::par::default_cutoff) + 50);
    const int sequential = mt::fold(v, 0, std::plus<>{});
    EXPECT_EQ(mt::reduce(v, 0, std::plus<>{}), sequential);
    EXPECT_EQ(mt::reduce(v), sequential);
    EXPECT_EQ(mt::reduce(std::vector<int>{}, 5, std::plus<>{}), 5);
}

TEST(Pfind, LeftmostMatchOrEnd) {
    std::vector<int> v{9, 1, 4, 1, 8};
    auto it = mt::pfind(v, 1);
    ASSERT_NE(it, v.end());
    EXPECT_EQ(it, v.begin() + 1);

    EXPECT_EQ(mt::pfind(v, 0), v.end());
    EXPECT_EQ(mt::pfind_if(v, [](int x) { return x > 7; }), v.begin());
}

TEST(Pfind, EmptyAndAboveCutoff) {
    std::vector<int> empty;
    EXPECT_EQ(mt::pfind(empty, 1), empty.end());

    auto v = iota_n(static_cast<int>(mt::par::default_cutoff) + 200);
    v[100] = -1;
    v.back() = -1;
    auto it = mt::pfind(v, -1);
    ASSERT_NE(it, v.end());
    EXPECT_EQ(it, v.begin() + 100);
}

TEST(Psort, SmallAndReversed) {
    std::vector<int> v{3, 1, 4, 1, 5};
    mt::psort(v);
    EXPECT_EQ(v, (std::vector<int>{1, 1, 3, 4, 5}));

    auto r = iota_n(100);
    std::reverse(r.begin(), r.end());
    mt::psort(r);
    EXPECT_TRUE(std::is_sorted(r.begin(), r.end()));
}

TEST(Psort, AboveCutoffMatchesStdSort) {
    auto v = iota_n(static_cast<int>(mt::par::default_cutoff) + 300);
    std::reverse(v.begin(), v.end());
    auto expected = v;
    std::sort(expected.begin(), expected.end());
    mt::psort(v);
    EXPECT_EQ(v, expected);
}

TEST(PartialBucketsort, SortsArithmeticRange) {
    std::vector<int> v{9, 1, 8, 2, 7, 3, 0};
    mt::partial_bucketsort(v);
    EXPECT_EQ(v, (std::vector<int>{0, 1, 2, 3, 7, 8, 9}));

    auto big = iota_n(static_cast<int>(mt::par::default_cutoff) + 128);
    std::reverse(big.begin(), big.end());
    mt::partial_bucketsort(big);
    EXPECT_TRUE(std::is_sorted(big.begin(), big.end()));
}

TEST(PartialBucketsort, EqualElementsAreStableEnoughToStaySorted) {
    std::vector<int> v(64, 7);
    mt::partial_bucketsort(v);
    EXPECT_EQ(v, std::vector<int>(64, 7));
}
