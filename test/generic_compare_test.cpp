#include "stlpp.h"

#include "support/fixtures.h"
#include "support/test_concepts.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <typename I0, typename I1, typename R>
std::pair<I0, I1> manual_find_mismatch(I0 f0, I0 l0, I1 f1, I1 l1, R r)
{
    while (f0 != l0 && f1 != l1 && std::invoke(r, *f0, *f1)) {
        ++f0;
        ++f1;
    }
    return {f0, f1};
}

template <typename Container>
class FindMismatchGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(FindMismatchGeneric, mt::test::EopCoreContainers);

TYPED_TEST(FindMismatchGeneric, MatchesManualEqualOracle) {
    auto a = mt::test::make_iota_container<TypeParam>(6);
    auto b = mt::test::make_iota_container<TypeParam>(6);
    const auto a_last = mt::test::filled_end(a, 6);
    const auto b_last = mt::test::filled_end(b, 6);
    const auto oracle =
        manual_find_mismatch(a.begin(), a_last, b.begin(), b_last, std::equal_to<>{});
    const auto got =
        mt::find_mismatch(a.begin(), a_last, b.begin(), b_last, std::equal_to<>{});
    EXPECT_EQ(oracle, got);

    if constexpr (!mt::test::is_std_array_v<TypeParam>) {
        if (!a.empty()) {
            *std::next(a.begin()) = 99;
        }
    }
    const auto oracle2 =
        manual_find_mismatch(a.begin(), a_last, b.begin(), b_last, std::equal_to<>{});
    const auto got2 =
        mt::find_mismatch(a.begin(), a_last, b.begin(), b_last, std::equal_to<>{});
    EXPECT_EQ(oracle2, got2);
}

template <typename Container>
class LexicographicalGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(LexicographicalGeneric, mt::test::EopCoreContainers);

TYPED_TEST(LexicographicalGeneric, EqualMatchesStdEqual) {
    constexpr int n = 5;
    auto a = mt::test::make_iota_container<TypeParam>(n);
    auto b = mt::test::make_iota_container<TypeParam>(n);
    const auto a_last = mt::test::filled_end(a, n);
    const auto b_last = mt::test::filled_end(b, n);
    EXPECT_TRUE(mt::lexicographical_equal(a.begin(), a_last, b.begin(), b_last));
    EXPECT_EQ(mt::lexicographical_equal(a.begin(), a_last, b.begin(), b_last),
              std::equal(a.begin(), a_last, b.begin(), b_last));
}

TYPED_TEST(LexicographicalGeneric, LessMatchesStdLexicographicalCompare) {
    constexpr int n = 6;
    auto shorter = mt::test::make_iota_container<TypeParam>(n - 1);
    auto longer = mt::test::make_iota_container<TypeParam>(n);
    const auto s_last = mt::test::filled_end(shorter, n - 1);
    const auto l_last = mt::test::filled_end(longer, n);
    EXPECT_EQ(mt::lexicographical_less(shorter.begin(), s_last, longer.begin(), l_last),
              std::lexicographical_compare(shorter.begin(), s_last, longer.begin(), l_last));
    EXPECT_EQ(mt::lexicographical_less(longer.begin(), l_last, shorter.begin(), s_last),
              std::lexicographical_compare(longer.begin(), l_last, shorter.begin(), s_last));
}

template <typename Container>
class OrderPredicateGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(OrderPredicateGeneric, mt::test::EopCoreContainers);

TYPED_TEST(OrderPredicateGeneric, StrictlyIncreasingOnIota) {
    constexpr int n = 6;
    auto c = mt::test::make_iota_container<TypeParam>(n);
    const auto last = mt::test::filled_end(c, n);
    EXPECT_TRUE(mt::strictly_increasing_range(c.begin(), last));
    EXPECT_TRUE(mt::relation_preserving(c.begin(), last, std::less<>{}));
    EXPECT_TRUE(mt::increasing_range(c.begin(), c.begin(), std::less<>{}));
}

TYPED_TEST(OrderPredicateGeneric, IncreasingAllowsPlateaus) {
    TypeParam c{};
    if constexpr (mt::test::is_std_array_v<TypeParam>) {
        c = {0, 0, 1, 2, 2, 3, 4, 5};
    } else {
        c = {0, 0, 1, 2, 2, 3};
    }
    const int n = mt::test::is_std_array_v<TypeParam> ? 8 : 6;
    const auto last = mt::test::filled_end(c, n);
    EXPECT_TRUE(mt::increasing_range(c.begin(), last));
    EXPECT_FALSE(mt::strictly_increasing_range(c.begin(), last));
    const auto mismatch = mt::find_adjacent_mismatch(c.begin(), last, std::less<>{});
    EXPECT_NE(mismatch, last);
}

TYPED_TEST(OrderPredicateGeneric, PartitionedMatchesSuffixTrueLaw) {
    constexpr int n = 6;
    auto c = mt::test::make_iota_container<TypeParam>(n);
    const auto last = mt::test::filled_end(c, n);
    // EoP: once `pred` is true, it stays true (suffix partition). Not std::is_partitioned.
    const auto pred = [](int x) { return x >= 3; };
    EXPECT_TRUE(mt::partitioned(c.begin(), last, pred));
    const auto first_true = std::find_if(c.begin(), last, pred);
    const auto after_true_run = std::find_if_not(first_true, last, pred);
    EXPECT_EQ(after_true_run, last);
}

template <typename Container>
class EqualRangeGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(EqualRangeGeneric, mt::test::EopCoreContainers);

TYPED_TEST(EqualRangeGeneric, CountedRangeWidthMatchesManualScan) {
    std::vector<int> sorted{0, 1, 1, 2, 3, 3, 3, 4};
    const int n = static_cast<int>(sorted.size());
    for (int value : {1, 3, 0, 4, 9}) {
        const auto [lo, hi] = mt::equal_range_n(sorted.begin(), n, value);
        const int manual = static_cast<int>(std::count(sorted.begin(), sorted.end(), value));
        EXPECT_EQ(std::distance(lo, hi), manual);
        for (auto it = lo; it != hi; ++it) {
            EXPECT_EQ(*it, value);
        }
    }
}

TYPED_TEST(EqualRangeGeneric, BoundedEqualRangeMatchesCounted) {
    std::vector<int> sorted{0, 1, 1, 2, 3, 3, 3, 4};
    const int value = 3;
    const auto bounded = mt::equal_range(sorted.begin(), sorted.end(), value);
    const auto counted = mt::equal_range_n(sorted.begin(), static_cast<int>(sorted.size()), value);
    EXPECT_EQ(bounded, counted);
}

TEST(CompareExtras, AdjacentMismatchForwardMatchesInputOnVector) {
    std::vector<int> v{1, 2, 2, 3};
    const auto input = mt::find_adjacent_mismatch(v.begin(), v.end(), std::less<>{});
    const auto forward =
        mt::find_adjacent_mismatch_forward(v.begin(), v.end(), std::less<>{});
    EXPECT_EQ(input, forward);
}

TEST(CompareExtras, LexicographicalEquivalentUsesRelation) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{1, 2, 5};
    const auto same_parity = [](int x, int y) { return (x % 2) == (y % 2); };
    EXPECT_TRUE(mt::lexicographical_equivalent(a.begin(), a.end(), b.begin(), b.end(), same_parity));
    EXPECT_FALSE(mt::lexicographical_equal(a.begin(), a.end(), b.begin(), b.end()));
}

}  // namespace
