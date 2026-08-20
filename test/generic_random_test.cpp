#include "stlpp.h"

#include "support/eop_postconditions.h"

#include <numeric>
#include <random>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(RandomGeneric, SeededValuesStayInClosedBounds) {
    std::mt19937_64 rng{42};
    std::vector<int> values(200, -1);
    mt::random(values, 3, 9, rng);
    mt::test::expect_random_in_closed_bounds(values, 3, 9);
}

TEST(RandomGeneric, SwappedBoundsStillValid) {
    std::mt19937_64 rng{7};
    std::vector<int> values(50);
    mt::random(values, 10, 1, rng);
    mt::test::expect_random_in_closed_bounds(values, 10, 1);
}

TEST(RandomGeneric, EqualBoundsFillConstant) {
    std::vector<int> values(8, 0);
    mt::random(values, 4, 4);
    EXPECT_EQ(values, std::vector<int>(8, 4));
}

TEST(RandomGeneric, EmptyRangeIsNoOp) {
    std::vector<int> values;
    EXPECT_NO_THROW(mt::random(values, 0, 10));
    EXPECT_TRUE(values.empty());
}

TEST(RandomIotaGeneric, ProducesPermutation) {
    std::mt19937_64 rng{123};
    std::vector<int> values(16, -1);
    mt::random_iota(values, rng);
    mt::test::expect_random_iota_permutation(values);
}

TEST(RandomIotaGeneric, EmptyAndSingleElement) {
    std::vector<int> empty;
    mt::random_iota(empty);
    EXPECT_TRUE(empty.empty());

    std::vector<int> one{99};
    mt::random_iota(one);
    EXPECT_EQ(one.front(), 0);
}

TEST(RandomIotaGeneric, ShuffledWithSeededRng) {
    std::mt19937_64 rng{2026};
    std::vector<int> values(32);
    mt::random_iota(values, rng);
    mt::test::expect_random_iota_permutation(values);
    std::vector<int> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_NE(values, sorted);
}

}  // namespace
