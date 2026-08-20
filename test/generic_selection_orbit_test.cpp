#include "stlpp.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(SelectGeneric, Select02And12AreMinMax) {
    const int a = 3;
    const int b = 1;
    EXPECT_EQ(&mt::select_0_2(a, b), &b);
    EXPECT_EQ(&mt::select_1_2(a, b), &a);
    EXPECT_EQ(mt::select_0_2(a, b), 1);
    EXPECT_EQ(mt::select_1_2(a, b), 3);
}

TEST(SelectGeneric, Select13IsMedianOfThree) {
    EXPECT_EQ(mt::select_1_3(1, 2, 3), 2);
    EXPECT_EQ(mt::select_1_3(3, 1, 2), 2);
    EXPECT_EQ(mt::select_1_3(2, 3, 1), 2);
}

TEST(SelectGeneric, Median5MatchesSortedMiddleOnAllPermutations) {
    std::array<int, 5> vals{1, 2, 3, 4, 5};
    do {
        const int m =
            mt::median_5(vals[0], vals[1], vals[2], vals[3], vals[4], std::less<>{});
        EXPECT_EQ(m, 3);
    } while (std::next_permutation(vals.begin(), vals.end()));
}

TEST(SelectGeneric, Select14IsSecondSmallest) {
    EXPECT_EQ(mt::select_1_4(4, 1, 3, 2), 2);
    EXPECT_EQ(mt::select_1_4(1, 2, 3, 4), 2);
}

TEST(OrbitExtras, PowerUnaryAndDistance) {
    auto inc = [](int x) { return x + 1; };
    EXPECT_EQ(mt::power_unary(0, 5, inc), 5);
    EXPECT_EQ(mt::transformation_distance(0, 5, inc), 5u);
}

TEST(OrbitExtras, TerminatingAndConnection) {
    auto inc = [](int x) { return x + 1; };
    auto defined = [](int x) { return x < 10; };
    EXPECT_TRUE(mt::terminating(0, inc, defined));
    EXPECT_EQ(mt::collision_point(0, inc, defined), 10);
    EXPECT_EQ(mt::connection_point(0, inc, defined), 10);
}

TEST(OrbitExtras, CircularModOrbit) {
    auto f = [](int x) { return (x + 1) % 5; };
    EXPECT_TRUE(mt::circular(0, f));
    EXPECT_EQ(mt::connection_point(0, f), 0);
    // rho: 0 -> 1 -> 2 -> 3 -> 2 ...
    auto rho = [](int x) {
        static constexpr int next[] = {1, 2, 3, 2};
        return next[x];
    };
    EXPECT_FALSE(mt::circular(0, rho));
    EXPECT_EQ(mt::connection_point(0, rho), 2);
}

TEST(QuantifierExtras, CountNotAndCountIfNot) {
    std::vector<int> v{1, 2, 2, 3, 2};
    EXPECT_EQ(mt::count_not(v.begin(), v.end(), 2), 2);
    EXPECT_EQ(mt::count_if_not(v, [](int x) { return x == 2; }), 2);
}

TEST(FindExtras, FindLastAndUnguarded) {
    std::vector<int> v{1, 2, 3, 4};
    EXPECT_EQ(*mt::find_last(v.begin(), v.end()), 4);
    EXPECT_EQ(mt::find_last(v.begin(), v.begin()), v.begin());
    auto it = mt::find_if_unguarded(v.begin(), [](int x) { return x == 3; });
    EXPECT_EQ(*it, 3);
    auto jt = mt::find_if_not_unguarded(v.begin(), [](int x) { return x < 3; });
    EXPECT_EQ(*jt, 3);
}

TEST(ReduceNonemptyGeneric, MatchesFoldWithoutInit) {
    std::vector<int> v{1, 2, 3, 4};
    EXPECT_EQ(mt::reduce_nonempty(v.begin(), v.end(), std::plus<>{}), 10);
}

}  // namespace
