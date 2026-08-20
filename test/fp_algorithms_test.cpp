// Behavioral examples and edge cases not asserted in test/generic_*_test.cpp law suites.

#include "stlpp.h"

#include <functional>
#include <vector>

#include <gtest/gtest.h>

TEST(Power, MultiplyAndPlusAndIdentity)
{
    EXPECT_EQ(mt::power(2, 10, std::multiplies<int>{}), 1024);
    EXPECT_EQ(mt::power(3, 5, std::plus<int>{}), 15);
    EXPECT_EQ(mt::power(2, 0, std::multiplies<int>{}, 1), 1);
    EXPECT_EQ(mt::power(2, 0, std::multiplies<int>{}), 2);
    EXPECT_TRUE(mt::even(4));
    EXPECT_FALSE(mt::even(3));
}

TEST(Scan, InclusiveAndExclusive)
{
    EXPECT_EQ(mt::scan(std::vector{1, 2, 3, 4}, std::plus<>{}), (std::vector<int>{1, 3, 6, 10}));
    EXPECT_EQ(mt::exclusive_scan(std::vector{1, 2, 3}, 0, std::plus<>{}),
              (std::vector<int>{0, 1, 3}));
    EXPECT_TRUE(mt::scan(std::vector<int>{}, std::plus<>{}).empty());
}

TEST(Orbit, CycleAndTerminating)
{
    auto wrap = [](int x) { return (x + 1) % 5; };
    const int c = mt::collision_point(0, wrap);
    EXPECT_GE(c, 0);
    EXPECT_LT(c, 5);

    auto inc = [](int x) { return x + 1; };
    auto defined = [](int x) { return x < 10; };
    EXPECT_EQ(mt::collision_point(0, inc, defined), 10);
}

TEST(LowerBoundN, FirstNotLess)
{
    std::vector<int> v{1, 2, 2, 4};
    EXPECT_EQ(mt::lower_bound_n(v.begin(), 4, 2), v.begin() + 1);
    EXPECT_EQ(mt::upper_bound_n(v.begin(), 4, 2), v.begin() + 3);
    EXPECT_EQ(mt::lower_bound_n(v.begin(), v.end(), 99, 4), v.begin() + 3);
    EXPECT_EQ(mt::upper_bound_n(v.begin(), v.end(), 99, 2), v.begin() + 3);
}

TEST(ListOps, GroupByParityAndSinglePermutationStep)
{
    auto same_parity = [](int a, int b) { return (a % 2) == (b % 2); };
    auto by_parity = mt::group_by(std::vector{1, 3, 2, 4, 5}, same_parity);
    ASSERT_EQ(by_parity.size(), 3u);
    EXPECT_EQ(by_parity[0], (std::vector<int>{1, 3}));
    EXPECT_EQ(by_parity[1], (std::vector<int>{2, 4}));
    EXPECT_EQ(by_parity[2], (std::vector<int>{5}));

    std::vector<int> perm{1, 2, 3};
    EXPECT_TRUE(mt::next_permutation(perm));
    EXPECT_EQ(perm, (std::vector<int>{1, 3, 2}));
}
