#include "stlpp.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(ReduceBalancedGeneric, MatchesAssociativeFold)
{
    std::vector<int> v(50);
    std::iota(v.begin(), v.end(), 0);
    const int expected = std::accumulate(v.begin(), v.end(), 0);
    EXPECT_EQ(mt::reduce_balanced(v.begin(), v.end(), std::plus<>{}, 0), expected);
    EXPECT_EQ(mt::reduce_balanced(v, std::plus<>{}, 0), expected);
}

TEST(SortNAdaptiveGeneric, MatchesSortNWithSmallAndLargeBuffer)
{
    std::vector<int> a{9, 1, 8, 2, 7, 3, 0, 4};
    std::vector<int> b = a;
    std::vector<int> c = a;
    std::vector<int> tiny(2);
    std::vector<int> big((a.size() + 1) / 2);
    (void)mt::sort_n(a.begin(), static_cast<int>(a.size()));
    (void)mt::sort_n_adaptive(b.begin(), static_cast<int>(b.size()), tiny.begin(), 2);
    (void)mt::sort_n_adaptive(c.begin(), static_cast<int>(c.size()), big.begin(),
                              static_cast<int>(big.size()));
    EXPECT_EQ(b, a);
    EXPECT_EQ(c, a);
    EXPECT_TRUE(std::is_sorted(a.begin(), a.end()));
}

TEST(AdvancedSortNGeneric, MatchesSortN)
{
    std::vector<int> a{5, 4, 3, 2, 1, 0, 9, 8, 7, 6};
    std::vector<int> b = a;
    (void)mt::advanced_sort_n(a.begin(), static_cast<int>(a.size()));
    (void)mt::sort_n(b.begin(), static_cast<int>(b.size()));
    EXPECT_EQ(a, b);
}

TEST(ReverseNAdaptiveGeneric, MatchesReverseN)
{
    std::vector<int> a{1, 2, 3, 4, 5, 6};
    std::vector<int> b = a;
    std::vector<int> buf(6);
    (void)mt::reverse_n(a.begin(), 6);
    (void)mt::reverse_n_adaptive(b.begin(), 6, buf.begin(), 6);
    EXPECT_EQ(a, b);
    std::vector<int> c{1, 2, 3, 4, 5, 6};
    (void)mt::reverse_n_adaptive(c.begin(), 6, buf.begin(), 2);
    EXPECT_EQ(c, a);
}

TEST(CycleGeneric, CycleFromRotatesOneOrbit)
{
    std::vector<int> v{0, 1, 2, 3};
    // from-permutation for rotate left by 1: i -> i+1, last -> first
    auto from = [&](auto it) {
        if (it + 1 == v.end()) {
            return v.begin();
        }
        return it + 1;
    };
    mt::cycle_from(v.begin(), from);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 0}));
}

TEST(MergeCopyBackwardGeneric, MatchesForwardMerge)
{
    std::vector<int> a{1, 3, 5};
    std::vector<int> b{0, 2, 4};
    std::vector<int> out(6, -1);
    std::vector<int> expect;
    std::merge(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(expect));
    (void)mt::merge_copy_backward(a.begin(), a.end(), b.begin(), b.end(), out.end());
    EXPECT_EQ(out, expect);
}

}  // namespace
