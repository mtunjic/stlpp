#include "stlpp.h"

#include "support/container_postconditions.h"
#include "support/eop_postconditions.h"
#include "support/fixtures.h"
#include "support/test_concepts.h"

#include <algorithm>
#include <list>
#include <numeric>
#include <random>
#include <vector>

#include <gtest/gtest.h>

namespace {

std::vector<int> large_iota()
{
    return mt::test::iota_vector(static_cast<int>(mt::par::default_cutoff) + 128);
}

}  // namespace

TEST(ParallelMapGeneric, MatchesElementwiseAboveCutoff)
{
    auto v = large_iota();
    const auto f = [](int x) { return x * 3 + 1; };
    auto out = mt::map(v, f);
    mt::test::expect_map_elementwise(v, out, f);
}

TEST(ParallelMapGeneric, SequentialForwardListStillCorrect)
{
    std::list<int> lst{1, 2, 3};
    std::vector<int> out;
    mt::map(lst.begin(), lst.end(), std::back_inserter(out), [](int x) { return x + 10; });
    mt::test::expect_map_elementwise(lst, out, [](int x) { return x + 10; });
}

TEST(ParallelReduceGeneric, AssociativePlusMatchesFoldAboveCutoff)
{
    auto v = large_iota();
    mt::test::check_associative_spot(std::plus<>{}, 1, 2, 3);
    mt::test::expect_parallel_reduce_matches_fold(v, 0, std::plus<>{});
}

TEST(ParallelReduceGeneric, EmptyReturnsInit)
{
    EXPECT_EQ(mt::reduce(std::vector<int>{}, 42, std::plus<>{}), 42);
}

TEST(ParallelPsortGeneric, MatchesStdSortAboveCutoff)
{
    auto v = large_iota();
    std::shuffle(v.begin(), v.end(), std::mt19937{99});
    mt::psort(v);
    mt::test::expect_sorted_prefix(v.begin(), v.end());
    mt::test::expect_sorted_like_std(v);
}

TEST(ParallelPsortGeneric, PartialBucketsortArithmetic)
{
    auto v = large_iota();
    std::shuffle(v.begin(), v.end(), std::mt19937{7});
    mt::partial_bucketsort(v);
    mt::test::expect_sorted_prefix(v.begin(), v.end());
}

TEST(ParallelPfindGeneric, LeftmostEvenAboveCutoff)
{
    auto v = large_iota();
    v[static_cast<std::size_t>(mt::par::default_cutoff) + 10] = 999999;
    v[static_cast<std::size_t>(mt::par::default_cutoff) + 50] = 888888;
    v[3] = 2;
    mt::test::expect_pfind_value_leftmost(v, 2);
    mt::test::expect_pfind_leftmost(v, [](int x) { return x % 2 == 0; });
}

TEST(ParallelPfindGeneric, NoMatchReturnsEnd)
{
    auto v = large_iota();
    EXPECT_EQ(mt::pfind(v, -1), v.end());
    EXPECT_EQ(mt::pfind_if(v, [](int) { return false; }), v.end());
}

TEST(ParallelReduceGeneric, FoldIsCanonicalForNonAssociativeOps)
{
    auto v = large_iota();
    mt::test::expect_fold_matches_manual(v, 0, std::minus<>{});
}
