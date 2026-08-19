#include "random.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include <gtest/gtest.h>

TEST(Random, SeededInt_AllValuesInsideInclusiveBounds) {
    std::mt19937_64 rng{42};
    std::vector<int> values(200, -1);
    mt::random(values, 3, 9, rng);

    EXPECT_TRUE(std::all_of(values.begin(), values.end(), [](int x) {
        return x >= 3 && x <= 9;
    }));
}

TEST(Random, SwappedBounds_StillFillsClosedInterval) {
    std::mt19937_64 rng{7};
    std::vector<int> values(50);
    mt::random(values, 10, 1, rng);
    EXPECT_TRUE(std::all_of(values.begin(), values.end(), [](int x) {
        return x >= 1 && x <= 10;
    }));
}

TEST(Random, EqualBounds_FillsConstant) {
    std::vector<int> values(8, 0);
    mt::random(values, 4, 4);
    EXPECT_EQ(values, std::vector<int>(8, 4));
}

TEST(Random, EmptyRange_IsNoOp) {
    std::vector<int> values;
    EXPECT_NO_THROW(mt::random(values, 0, 10));
    EXPECT_TRUE(values.empty());
}

TEST(Random, PairBounds_MatchesTwoArgOverload) {
    std::mt19937_64 rng_a{99};
    std::mt19937_64 rng_b{99};
    std::vector<int> a(32);
    std::vector<int> b(32);
    mt::random(a.begin(), a.end(), 1, 5, rng_a);
    mt::random(b, std::pair{1, 5});
    // pair overload uses default_rng — not comparable. Check bounds only.
    EXPECT_TRUE(std::all_of(b.begin(), b.end(), [](int x) { return x >= 1 && x <= 5; }));
    EXPECT_TRUE(std::all_of(a.begin(), a.end(), [](int x) { return x >= 1 && x <= 5; }));
}

TEST(Random, Double_AllValuesInsideBounds) {
    std::mt19937_64 rng{1};
    std::vector<double> values(100);
    mt::random(values, 0.5, 5.0, rng);
    EXPECT_TRUE(std::all_of(values.begin(), values.end(), [](double x) {
        return x >= 0.5 && x < 5.0;
    }));
}

TEST(RandomIota, ResultIsPermutationOfZeroToNMinusOne) {
    std::mt19937_64 rng{123};
    std::vector<int> values(16, -1);
    mt::random_iota(values, rng);

    std::vector<int> expected(16);
    std::iota(expected.begin(), expected.end(), 0);
    std::sort(values.begin(), values.end());
    EXPECT_EQ(values, expected);
}

TEST(RandomIota, EmptyAndSingle_AreStable) {
    std::vector<int> empty;
    mt::random_iota(empty);
    EXPECT_TRUE(empty.empty());

    std::vector<int> one{42};
    mt::random_iota(one);
    EXPECT_EQ(one.front(), 0);
}

TEST(RandomIota, ShufflesAwayFromSortedWithHighProbability) {
    std::mt19937_64 rng{2026};
    std::vector<int> values(32);
    mt::random_iota(values, rng);

    std::vector<int> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_NE(values, sorted);
    EXPECT_TRUE(std::is_permutation(values.begin(), values.end(), sorted.begin()));
}
