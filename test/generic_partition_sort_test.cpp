#include "stlpp.h"

#include "support/fixtures.h"
#include "support/test_concepts.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <typename Container>
class PartitionStableGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(PartitionStableGeneric, mt::test::EopCoreContainers);

TYPED_TEST(PartitionStableGeneric, PreservesRelativeOrderOfMatches) {
    TypeParam c{};
    if constexpr (mt::test::is_std_array_v<TypeParam>) {
        c = {1, 2, 3, 4, 5, 6, 7, 8};
    } else {
        c = {1, 2, 3, 4, 5, 6};
    }
    const int n = mt::test::is_std_array_v<TypeParam> ? 8 : 6;
    const auto last = mt::test::filled_end(c, n);
    const auto odd = [](int x) { return x % 2 != 0; };
    std::vector<int> odds_before;
    for (auto it = c.begin(); it != last; ++it) {
        if (odd(*it)) {
            odds_before.push_back(*it);
        }
    }
    const auto mid = mt::partition_stable(c.begin(), last, odd);
    EXPECT_TRUE(std::is_partitioned(c.begin(), last, odd));
    std::vector<int> odds_after(c.begin(), mid);
    EXPECT_EQ(odds_after, odds_before);
}

TEST(PartitionVariants, SemistableAndForwardPartition) {
    std::vector<int> a{1, 2, 3, 4, 5, 6};
    std::vector<int> b = a;
    const auto odd = [](int x) { return x % 2 != 0; };
    const auto ma = mt::partition_semistable(a.begin(), a.end(), odd);
    const auto mb = mt::partition_forward(b.begin(), b.end(), odd);
    // EoP semistable: false* then true* (suffix-true / `mt::partitioned`).
    EXPECT_TRUE(mt::partitioned(a.begin(), a.end(), odd));
    EXPECT_EQ(ma, std::find_if(a.begin(), a.end(), odd));
    // std / forward partition: true* then false*.
    EXPECT_TRUE(std::is_partitioned(b.begin(), b.end(), odd));
    EXPECT_EQ(std::count_if(a.begin(), a.end(), odd), std::count_if(b.begin(), mb, odd));
    (void)ma;
}

TEST(PartitionVariants, BidirectionalAndBufferStable) {
    std::vector<int> a{9, 0, 1, 4, 7, 2};
    std::vector<int> b = a;
    const auto lt5 = [](int x) { return x < 5; };
    (void)mt::partition_bidirectional(a.begin(), a.end(), lt5);
    std::vector<int> buf(a.size());
    (void)mt::partition_stable_with_buffer(b.begin(), b.end(), buf.begin(), lt5);
    EXPECT_TRUE(std::is_partitioned(a.begin(), a.end(), lt5));
    EXPECT_TRUE(std::is_partitioned(b.begin(), b.end(), lt5));
}

TEST(RemoveIfGeneric, MatchesStdRemoveIfOracle) {
    std::vector<int> a{1, 2, 3, 4, 5, 6};
    std::vector<int> b = a;
    const auto even = [](int x) { return x % 2 == 0; };
    const auto pa = mt::remove_if(a.begin(), a.end(), even);
    const auto pb = std::remove_if(b.begin(), b.end(), even);
    EXPECT_EQ(std::vector<int>(a.begin(), pa), std::vector<int>(b.begin(), pb));
}

TEST(FillGeneric, FillAndFillNWriteValue) {
    std::vector<int> v(5, -1);
    EXPECT_EQ(mt::fill(v.begin(), v.end(), 7), v.end());
    EXPECT_TRUE(std::all_of(v.begin(), v.end(), [](int x) { return x == 7; }));
    auto mid = mt::fill_n(v.begin(), 2, 3);
    EXPECT_EQ(mid, v.begin() + 2);
    EXPECT_EQ(v, (std::vector<int>{3, 3, 7, 7, 7}));
}

TEST(SortNGeneric, SortNIsIdempotentAndSorted) {
    std::vector<int> v{9, 1, 8, 2, 7, 3, 0};
    const auto end = mt::sort_n(v.begin(), static_cast<int>(v.size()));
    EXPECT_EQ(end, v.end());
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
    auto again = v;
    (void)mt::sort_n(again.begin(), static_cast<int>(again.size()));
    EXPECT_EQ(again, v);
}

TEST(MergeNWithBufferGeneric, MergesTwoSortedHalves) {
    std::vector<int> v{1, 3, 5, 0, 2, 4};
    std::vector<int> buf(3);
    const auto end = mt::merge_n_with_buffer(v.begin(), 3, v.begin() + 3, 3, buf.begin());
    EXPECT_EQ(end, v.end());
    EXPECT_EQ(v, (std::vector<int>{0, 1, 2, 3, 4, 5}));
}

TEST(SortNWithBufferGeneric, MatchesSortN) {
    std::vector<int> a{4, 1, 3, 2, 0};
    std::vector<int> b = a;
    std::vector<int> buf((a.size() + 1) / 2);
    (void)mt::sort_n_with_buffer(a.begin(), static_cast<int>(a.size()), buf.begin());
    (void)mt::sort_n(b.begin(), static_cast<int>(b.size()));
    EXPECT_EQ(a, b);
}

}  // namespace
