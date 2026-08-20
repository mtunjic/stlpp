#include "stlpp.h"

#include "support/eop_postconditions.h"

#include <optional>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(UnfoldGeneric, UnfoldStepMatchesManual)
{
    auto step = [](int n) -> std::optional<std::pair<int, int>> {
        if (n <= 0) {
            return std::nullopt;
        }
        return std::pair{n, n - 1};
    };
    EXPECT_EQ(mt::unfold(step, 3), (std::vector<int>{3, 2, 1}));
}

TEST(IterateNGeneric, BuildsGeometricSequence)
{
    const auto f = [](int x) { return x * 2; };
    EXPECT_EQ(mt::iterate_n(5, 1, f), (std::vector<int>{1, 2, 4, 8, 16}));
    EXPECT_TRUE(mt::iterate_n(0, 1, f).empty());
    EXPECT_TRUE(mt::iterate_n(-3, 1, f).empty());
}

TEST(AdjacentDifferenceGeneric, MatchesStdPartial)
{
    const std::vector<int> in{1, 3, 6, 10};
    EXPECT_EQ(mt::adjacent_difference(in), (std::vector<int>{1, 2, 3, 4}));
}

TEST(PartitionCopyGeneric, SplitsWithoutMutatingSource)
{
    const std::vector<int> src{1, 2, 3, 4};
    const auto [evens, odds] = mt::partition_copy(src, [](int x) { return x % 2 == 0; });
    mt::test::expect_ranges_equal(src, std::vector<int>{1, 2, 3, 4});
    EXPECT_EQ(evens, (std::vector<int>{2, 4}));
    EXPECT_EQ(odds, (std::vector<int>{1, 3}));
}

TEST(CartesianProductGeneric, SizeIsProductOfLengths)
{
    const auto prod = mt::cartesian_product(std::vector{1, 2}, std::vector{'a', 'b'});
    ASSERT_EQ(prod.size(), 4u);
    EXPECT_EQ(prod[0], (std::pair<int, char>{1, 'a'}));
    EXPECT_EQ(prod[3], (std::pair<int, char>{2, 'b'}));
}

TEST(NextPermutationGeneric, WalksFactorialDistinctOrders)
{
    std::vector<int> perm{1, 2, 3};
    std::size_t count = 1;
    while (mt::next_permutation(perm)) {
        ++count;
    }
    EXPECT_EQ(count, 6u);
    EXPECT_EQ(perm, (std::vector<int>{1, 2, 3}));
}

}  // namespace
