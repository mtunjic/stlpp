#include "stlpp.h"

#include "support/eop_postconditions.h"
#include "support/fixtures.h"
#include "support/test_concepts.h"

#include <algorithm>
#include <array>
#include <list>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <typename Container>
class CopyNGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(CopyNGeneric, mt::test::EopCoreContainers);

TYPED_TEST(CopyNGeneric, PreservesPrefix)
{
    constexpr int fill = 6;
    auto src = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(src, fill);
    std::vector<int> dst(6, -1);
    const int n = 4;
    auto [in, out] = mt::copy_n(src.begin(), last, n, dst.begin());
    EXPECT_EQ(std::distance(src.begin(), in), n);
    EXPECT_EQ(std::vector<int>(dst.begin(), dst.begin() + n), mt::test::iota_vector(n));
}

TYPED_TEST(CopyNGeneric, SentinelOverrunStopsAtEnd)
{
    constexpr int fill = 3;
    auto src = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(src, fill);
    const int logical = static_cast<int>(std::distance(src.begin(), last));
    std::vector<int> dst(static_cast<std::size_t>(logical), 0);
    auto [in, out] = mt::copy_n(src.begin(), last, 99, dst.begin());
    EXPECT_EQ(in, last);
    EXPECT_EQ(out, dst.begin() + logical);
}

template <typename Container>
class ZipNGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(ZipNGeneric, mt::test::EopCoreContainers);

TYPED_TEST(ZipNGeneric, LengthIsMinimum)
{
    constexpr int fill_a = 5;
    constexpr int fill_b = 3;
    auto a = mt::test::make_iota_container<TypeParam>(fill_a);
    auto b = mt::test::make_iota_container<TypeParam>(fill_b, 10);
    const auto last_a = mt::test::filled_end(a, fill_a);
    const auto last_b = mt::test::filled_end(b, fill_b);
    std::vector<std::pair<int, int>> out;
    (void)mt::zip_n(a.begin(), last_a, b.begin(), last_b, 99, std::back_inserter(out));
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], (std::pair<int, int>{0, 10}));
    EXPECT_EQ(out[2], (std::pair<int, int>{2, 12}));
}

template <typename Container>
class SwapRangesNGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(SwapRangesNGeneric, mt::test::EopCoreContainers);

TYPED_TEST(SwapRangesNGeneric, InvolutionOnPrefix)
{
    constexpr int fill = 5;
    auto a = mt::test::make_iota_container<TypeParam>(fill);
    auto b = mt::test::make_iota_container<TypeParam>(fill, 100);
    const auto last_a = mt::test::filled_end(a, fill);
    const auto last_b = mt::test::filled_end(b, fill);
    const auto a0 = mt::test::to_filled_vector(a, fill);
    const auto b0 = mt::test::to_filled_vector(b, fill);
    (void)mt::swap_ranges_n(a.begin(), last_a, b.begin(), last_b, 3);
    (void)mt::swap_ranges_n(a.begin(), last_a, b.begin(), last_b, 3);
    mt::test::expect_ranges_equal(mt::test::to_filled_vector(a, fill), a0);
    mt::test::expect_ranges_equal(mt::test::to_filled_vector(b, fill), b0);
}

template <typename Container>
class SkipTakeGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(SkipTakeGeneric, mt::test::EopCoreContainers);

TYPED_TEST(SkipTakeGeneric, SkipTakePartitionSize)
{
    const int fill = mt::test::effective_fill<TypeParam>(8);
    auto c = mt::test::make_iota_container<TypeParam>(fill);
    const auto n = fill;
    if (n < 4) {
        GTEST_SKIP() << "container too small";
    }
    auto skipped = mt::skip(c, 3);
    auto [a, b] = mt::take(c, 3);
    EXPECT_EQ(std::distance(skipped.begin(), skipped.end()), n - 3);
    EXPECT_EQ(std::distance(a, b), 3);
    EXPECT_EQ(*skipped.begin(), 3);
}

TYPED_TEST(SkipTakeGeneric, TakeWhileLength)
{
    std::vector<int> v{-1, -2, 3, 4};
    auto [a, b] = mt::take_while(v, [](int x) { return x < 0; });
    EXPECT_EQ(std::distance(a, b), 2);
}

template <typename Container>
class ReverseNGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(ReverseNGeneric, mt::test::EopCoreContainers);

TYPED_TEST(ReverseNGeneric, PrefixReverseBidirectional)
{
    if constexpr (std::bidirectional_iterator<typename TypeParam::iterator>) {
        constexpr int fill = 6;
        auto c = mt::test::make_iota_container<TypeParam>(fill);
        const auto last = mt::test::filled_end(c, fill);
        const auto orig = mt::test::to_filled_vector(c, fill);
        mt::reverse_n(c.begin(), last, 4);
        std::vector<int> expected = orig;
        std::reverse(expected.begin(), expected.begin() + 4);
        mt::test::expect_ranges_equal(mt::test::to_filled_vector(c, fill), expected);
    }
}

TEST(ReverseNGeneric, ForwardWithBuffer)
{
    std::list<int> lst{0, 1, 2, 3, 4};
    int buf[5]{};
    mt::reverse_n_with_buffer(lst.begin(), lst.end(), 5, buf);
    mt::test::expect_ranges_equal(lst, std::vector<int>{4, 3, 2, 1, 0});
}

template <typename Container>
class PartitionSortGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(PartitionSortGeneric, mt::test::EopCoreContainers);

TYPED_TEST(PartitionSortGeneric, PartitionPointOnSortedPredicate)
{
    constexpr int fill = 5;
    auto v = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(v, fill);
    auto p = mt::partition_point_n(v.begin(), last, 99, [](int x) { return x < 3; });
    mt::test::expect_partitioned_at(v.begin(), p, last, [](int x) { return x < 3; });
}

TYPED_TEST(PartitionSortGeneric, LowerUpperBoundMatchStd)
{
    if constexpr (std::random_access_iterator<typename TypeParam::iterator>) {
        constexpr int fill = 6;
        auto v = mt::test::make_iota_container<TypeParam>(fill);
        const auto last = mt::test::filled_end(v, fill);
        v[3] = 2;
        v[4] = 2;
        std::sort(v.begin(), last);
        const int key = 2;
        auto l = mt::lower_bound_n(v.begin(), last, 99, key);
        auto u = mt::upper_bound_n(v.begin(), last, 99, key);
        EXPECT_EQ(l, std::lower_bound(v.begin(), last, key));
        EXPECT_EQ(u, std::upper_bound(v.begin(), last, key));
    }
}

TYPED_TEST(PartitionSortGeneric, BinaryInsertionSortSorted)
{
    if constexpr (std::random_access_iterator<typename TypeParam::iterator>) {
        constexpr int fill = 6;
        auto v = mt::test::make_iota_container<TypeParam>(fill);
        const auto last = mt::test::filled_end(v, fill);
        std::shuffle(v.begin(), last, std::mt19937{123});
        mt::binary_insertion_sort_n(v.begin(), last, fill);
        mt::test::expect_sorted_prefix(v.begin(), last);
    }
}

template <typename Container>
class FoldScanGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(FoldScanGeneric, mt::test::EopCoreContainers);

TYPED_TEST(FoldScanGeneric, FoldMatchesManualPlus)
{
    constexpr int fill = 5;
    auto c = mt::test::make_iota_container<TypeParam>(fill);
    const auto slice = mt::test::to_filled_vector(c, fill);
    mt::test::expect_fold_matches_manual(slice, 0, std::plus<>{});
}

TYPED_TEST(FoldScanGeneric, InclusiveScanPrefixLaw)
{
    constexpr int fill = 4;
    auto in = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(in, fill);
    std::vector<int> out;
    mt::scan(in.begin(), last, std::back_inserter(out), std::plus<>{});
    mt::test::expect_inclusive_scan_prefix(mt::test::to_filled_vector(in, fill), out,
                                           std::plus<>{});
}

TYPED_TEST(FoldScanGeneric, ExclusiveScanPrefixLaw)
{
    constexpr int fill = 4;
    auto in = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(in, fill);
    std::vector<int> out;
    mt::exclusive_scan(in.begin(), last, std::back_inserter(out), 0, std::plus<>{});
    mt::test::expect_exclusive_scan_prefix(mt::test::to_filled_vector(in, fill), out, 0,
                                           std::plus<>{});
}

TEST(FoldScanGeneric, BinaryCounterMatchesLinearFold)
{
    mt::binary_counter<int, std::plus<>> c(std::plus<>{}, 0);
    for (int x : {1, 2, 3, 4}) {
        c.add(x);
    }
    EXPECT_EQ(c.reduce(), mt::fold(std::vector<int>{1, 2, 3, 4}, 0, std::plus<>{}));
}

TEST(IntegerPowerGcd, OddHalfPowerLaws)
{
    EXPECT_TRUE(mt::odd(3));
    EXPECT_FALSE(mt::odd(4));
    EXPECT_EQ(mt::half(7), 3);
    mt::test::expect_power_equals_repeat(2, 10, std::multiplies<>{}, 1);
    mt::test::expect_power_equals_repeat(3, 5, std::plus<>{}, 0);
}

TEST(IntegerPowerGcd, GcdDividesAndExtendedBezout)
{
    mt::test::expect_gcd_divides(48, 18, mt::stein_gcd(48, 18));
    mt::test::expect_extended_gcd_bezout(30, 12);
    mt::test::expect_extended_gcd_bezout(17, 13);
}

TEST(OrbitGeneric, TerminatingCollision)
{
    auto f = [](int x) { return x + 1; };
    auto p = [](int x) { return x < 5; };
    EXPECT_EQ(mt::collision_point(0, f, p), 5);
}

TEST(OrbitGeneric, CircularCollisionOnMod)
{
    auto f = [](int x) { return (x + 1) % 4; };
    const int cp = mt::collision_point(0, f);
    EXPECT_GE(cp, 0);
    EXPECT_LT(cp, 4);
}

TEST(IteratorOpsGeneric, AdvanceBounded)
{
    std::vector<int> v{0, 1, 2, 3};
    auto it = v.begin();
    mt::advance(it, v.end(), 99);
    EXPECT_EQ(it, v.end());
}

template <typename Container>
class FindIfNGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(FindIfNGeneric, mt::test::EopCoreContainers);

TYPED_TEST(FindIfNGeneric, FindsWithinCount)
{
    constexpr int fill = 6;
    auto c = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(c, fill);
    auto [it, left] = mt::find_if_n(c.begin(), last, 4, [](int x) { return x >= 3; });
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(left, 1);
}

TYPED_TEST(FindIfNGeneric, SentinelOverrunReturnsEnd)
{
    constexpr int fill = 3;
    auto c = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(c, fill);
    auto [it, left] = mt::find_if_n(c.begin(), last, 99, [](int) { return false; });
    EXPECT_EQ(it, last);
    EXPECT_GT(left, 0);
}

TEST(ListOpsGeneric, GroupByPreservesDistinctElements)
{
    std::vector<std::string> words{"aa", "ab", "bb", "bc"};
    auto groups = mt::group_by(words, [](const std::string& a, const std::string& b) {
        return !a.empty() && !b.empty() && a[0] == b[0];
    });
    ASSERT_EQ(groups.size(), 2u);
    EXPECT_EQ(groups[0], (std::vector<std::string>{"aa", "ab"}));
    EXPECT_EQ(groups[1], (std::vector<std::string>{"bb", "bc"}));
}

TEST(ListOpsGeneric, MapAccumCarriesState)
{
    auto f = [](int acc, int x) { return std::pair{acc + x, acc + x}; };
    auto [sum, prefixes] = mt::map_accum(std::vector{1, 2, 3}, 0, f);
    EXPECT_EQ(sum, 6);
    EXPECT_EQ(prefixes, (std::vector<int>{1, 3, 6}));
}

TEST(ListOpsGeneric, InnerProductStopsAtShorterRange)
{
    EXPECT_EQ(mt::inner_product(std::vector{1, 2, 3}, std::vector{4, 5}), 14);
}

template <typename Container>
class FindBackwardGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(FindBackwardGeneric, mt::test::EopCoreContainers);

TYPED_TEST(FindBackwardGeneric, FindsLastOccurrence)
{
    if constexpr (!std::bidirectional_iterator<typename TypeParam::iterator>) {
        GTEST_SKIP() << "requires bidirectional iterator";
    }
    TypeParam v{5, 3, 0, 1, 8, 0, 9};
    constexpr int fill = 7;
    const auto last = mt::test::filled_end(v, fill);
    const auto p = mt::find_backward(v.begin(), last, 0);
    ASSERT_NE(p, last);
    EXPECT_EQ(*p, 0);
    EXPECT_EQ(std::distance(v.begin(), p), 5);
}

TYPED_TEST(FindBackwardGeneric, CountedBackwardSearch)
{
    if constexpr (!std::bidirectional_iterator<typename TypeParam::iterator>) {
        GTEST_SKIP() << "requires bidirectional iterator";
    }
    TypeParam v{1, 2, 3, 4, 5};
    constexpr int fill = 5;
    const auto last = mt::test::filled_end(v, fill);
    const auto p = mt::find_backward_if_n(v.begin(), last, 2, [](int x) { return x % 2 == 0; });
    ASSERT_NE(p, last);
    EXPECT_EQ(*p, 4);
}

template <typename Container>
class RemoveIfNotGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(RemoveIfNotGeneric, mt::test::EopCoreContainers);

TYPED_TEST(RemoveIfNotGeneric, StableForwardCompaction)
{
    TypeParam v{9, 0, 1, 4, 7};
    constexpr int fill = 5;
    const auto last = mt::test::filled_end(v, fill);
    mt::test::expect_remove_if_not_law(v.begin(), last, [](int x) { return x < 5; });
}

TYPED_TEST(RemoveIfNotGeneric, BackwardCompaction)
{
    if constexpr (!std::bidirectional_iterator<typename TypeParam::iterator>) {
        GTEST_SKIP() << "requires bidirectional iterator";
    }
    TypeParam v{9, 0, 1, 4, 7};
    constexpr int fill = 5;
    const auto last = mt::test::filled_end(v, fill);
    mt::test::expect_remove_if_not_backward_law(v.begin(), last, [](int x) { return x < 5; });
}

TEST(SpanOpsGeneric, SpanIfBreakIfSplitAtLaws)
{
    std::vector<int> v{2, 4, 5, 6};
    mt::test::expect_span_if_law(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
    mt::test::expect_break_if_law(v.begin(), v.end(), [](int x) { return x >= 5; });
    mt::test::expect_split_at_law(v.begin(), v.end(), 2);
    mt::test::expect_split_at_law(v.begin(), v.end(), 99);
}

TEST(MapcatGeneric, MatchesManualConcatMap)
{
    const std::vector<int> in{1, 2, 3};
    const auto dup = [](int x) { return std::vector{x, x}; };
    mt::test::expect_mapcat_manual(in, dup);
    EXPECT_EQ(mt::flatten(std::vector<std::vector<int>>{{1, 2}, {}, {3}}),
              (std::vector<int>{1, 2, 3}));
}

template <typename Container>
class ApplyGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(ApplyGeneric, mt::test::EopCoreContainers);

TYPED_TEST(ApplyGeneric, MapsInPlace)
{
    constexpr int fill = 3;
    auto v = mt::test::make_iota_container<TypeParam>(fill, 1);
    const auto last = mt::test::filled_end(v, fill);
    mt::test::expect_apply_maps(v.begin(), last, [](int x) { return x * x; });
}

TYPED_TEST(ApplyGeneric, ApplyUntilStopsAtPredicate)
{
    constexpr int fill = 4;
    auto v = mt::test::make_iota_container<TypeParam>(fill, 1);
    const auto last = mt::test::filled_end(v, fill);
    mt::test::expect_apply_until_law(
        v.begin(), last, [](int x) { return x + 10; }, [](int x) { return x == 3; });
}

template <typename Container>
class QuantifiersGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(QuantifiersGeneric, mt::test::EopCoreContainers);

TYPED_TEST(QuantifiersGeneric, MatchStdAlgorithms)
{
    constexpr int fill = 6;
    auto c = mt::test::make_iota_container<TypeParam>(fill);
    const auto slice = mt::test::to_filled_vector(c, fill);
    mt::test::expect_quantifiers_match_std(slice, [](int x) { return x % 2 == 0; });
    mt::test::expect_quantifiers_match_std(slice, [](int) { return false; });
}

TYPED_TEST(QuantifiersGeneric, EmptyAllIsTrue)
{
    if constexpr (mt::test::is_std_array_v<TypeParam>) {
        GTEST_SKIP() << "std::array is never empty";
    } else {
        TypeParam empty{};
        EXPECT_TRUE(mt::all(empty, [](int) { return false; }));
    }
}

template <typename Container>
class SliceGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(SliceGeneric, mt::test::EopCoreContainers);

TYPED_TEST(SliceGeneric, ClipsStartAndCount)
{
    if constexpr (!std::random_access_iterator<typename TypeParam::iterator>) {
        GTEST_SKIP() << "requires random access";
    } else {
        TypeParam v{0, 1, 2, 3, 4};
        constexpr int fill = 5;
        const auto last = mt::test::filled_end(v, fill);
        mt::test::expect_slice_law(v.begin(), last, 1, 3);
        const auto sub = mt::slice(v, 1, 3);
        EXPECT_EQ(std::distance(sub.begin(), sub.end()), 3);
        EXPECT_EQ(*sub.begin(), 1);
        EXPECT_TRUE(mt::slice(v, 10, 3).empty());
    }
}

TEST(PartitionExtrasGeneric, GatherClustersPredicates)
{
    std::array<int, 6> a{9, 0, 1, 4, 0, 7};
    const auto pos = a.begin() + 3;
    mt::test::expect_gather_law(a.begin(), a.end(), pos, [](int x) { return x < 2; });
}

TEST(PartitionExtrasGeneric, SlideWindowBounds)
{
    std::vector<int> v{0, 1, 2, 3, 4, 5};
    const auto [lo, hi] = mt::slide(v.begin() + 1, v.begin() + 3, v.begin() + 5);
    mt::test::expect_ranges_equal(std::vector<int>(lo, hi), (std::vector<int>{1, 2}));
}

TEST(RangeOpsGeneric, PairwiseConcatGroupShiftLaws)
{
    EXPECT_EQ(mt::pairwise(std::vector<int>{1, 2, 3}),
              (std::vector<std::pair<int, int>>{{1, 2}, {2, 3}}));
    EXPECT_EQ(mt::concat(std::vector<int>{1, 2}, std::vector<int>{3}), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(mt::group(std::vector<int>{1, 1, 2, 2, 1}),
              (std::vector<std::vector<int>>{{1, 1}, {2, 2}, {1}}));
    std::vector<int> rot{1, 2, 3, 4, 5};
    mt::shift_by(rot, 2);
    EXPECT_EQ(rot, (std::vector<int>{3, 4, 5, 1, 2}));
}

TEST(TransposeGeneric, DoubleTransposeIsIdentity)
{
    const std::vector<std::vector<int>> m{{1, 2, 3}, {4, 5, 6}};
    mt::test::expect_transpose_roundtrip(m);
    EXPECT_EQ(mt::transpose(m), (std::vector<std::vector<int>>{{1, 4}, {2, 5}, {3, 6}}));
}

template <typename Container>
class IotaGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(IotaGeneric, mt::test::EopCoreContainers);

TYPED_TEST(IotaGeneric, FillsAscendingFromStart)
{
    constexpr int fill = 5;
    TypeParam v{};
    const auto last = mt::test::filled_end(v, fill);
    mt::test::expect_iota_law(v.begin(), last, 10);
}

TYPED_TEST(IotaGeneric, ReverseIotaCountsDown)
{
    constexpr int fill = 5;
    TypeParam v{};
    const auto last = mt::test::filled_end(v, fill);
    mt::test::expect_reverse_iota_law(v.begin(), last);
}

TEST(IotaShapesGeneric, HillAndValleyPatterns)
{
    std::vector<int> hill(5);
    mt::hill(hill.begin(), hill.end());
    EXPECT_EQ(hill, (std::vector<int>{0, 1, 2, 1, 0}));
    std::vector<int> valley(5);
    mt::valley(valley.begin(), valley.end());
    EXPECT_EQ(valley, (std::vector<int>{1, 0, 0, 1, 2}));
}

template <typename Container>
class ZipUnzipGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(ZipUnzipGeneric, mt::test::EopCoreContainers);

TYPED_TEST(ZipUnzipGeneric, UnzipInvertsZip)
{
    constexpr int fill = 4;
    auto a = mt::test::make_iota_container<TypeParam>(fill, 1);
    auto b = mt::test::make_iota_container<TypeParam>(fill, 10);
    const auto slice_a = mt::test::to_filled_vector(a, fill);
    const auto slice_b = mt::test::to_filled_vector(b, fill);
    mt::test::expect_zip_unzip_roundtrip(slice_a, slice_b);
}

TYPED_TEST(ZipUnzipGeneric, ZipWithAppliesOp)
{
    constexpr int fill = 3;
    auto a = mt::test::make_iota_container<TypeParam>(fill, 1);
    auto b = mt::test::make_iota_container<TypeParam>(fill, 10);
    const auto last_a = mt::test::filled_end(a, fill);
    const auto last_b = mt::test::filled_end(b, fill);
    std::vector<int> summed;
    mt::zip_with(a.begin(), last_a, b.begin(), last_b, std::back_inserter(summed), std::plus<>{});
    ASSERT_EQ(summed.size(), static_cast<std::size_t>(fill));
    for (int i = 0; i < fill; ++i) {
        EXPECT_EQ(summed[static_cast<std::size_t>(i)], (1 + i) + (10 + i));
    }
}

template <typename Container>
class MinMaxGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(MinMaxGeneric, mt::test::EopCoreContainers);

TYPED_TEST(MinMaxGeneric, BoundsAllElements)
{
    constexpr int fill = 7;
    auto c = mt::test::make_iota_container<TypeParam>(fill);
    const auto last = mt::test::filled_end(c, fill);
    mt::test::expect_min_max_law(c.begin(), last);
    const auto [lo, hi] = mt::min_max(c.begin(), last);
    EXPECT_EQ(*lo, 0);
    EXPECT_EQ(*hi, fill - 1);
}

TEST(PartitionExtrasGeneric, StablePartitionPositionByIndexMask)
{
    int a[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};
    const bool mask[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0};
    const auto mid = mt::stable_partition_position(std::begin(a), std::end(a), [&](int* it) {
        return mask[static_cast<std::size_t>(it - std::begin(a))];
    });
    EXPECT_EQ(std::vector<int>(std::begin(a), mid), (std::vector<int>{2, 4, 4, 2}));
    EXPECT_EQ(std::vector<int>(mid, std::end(a)), (std::vector<int>{1, 3, 5, 5, 3, 1}));
}

TEST(RangeOpsGeneric, IntercalateInsertsSeparator)
{
    const std::vector<char> letters{'a', 'b', 'c'};
    EXPECT_EQ(mt::intercalate(',', letters), (std::vector<char>{'a', ',', 'b', ',', 'c'}));
    std::string s;
    mt::intercalate(',', letters.begin(), letters.end(), std::back_inserter(s));
    EXPECT_EQ(s, "a,b,c");
}

TEST(PrefixSuffixGeneric, StartsAndEndsWithLaws)
{
    EXPECT_TRUE(mt::starts_with(std::string{"https://ex.com"}, std::string{"https://"}));
    EXPECT_TRUE(mt::ends_with(std::string{"file.cpp"}, std::string{".cpp"}));
    EXPECT_FALSE(mt::starts_with(std::string{"abc"}, std::string{"abcd"}));
    EXPECT_FALSE(mt::ends_with(std::string{"ab"}, std::string{"cab"}));
}

TEST(BorderGeneric, OneDimensionalFrameLaw)
{
    mt::test::expect_border_1d(std::vector<int>{1, 2, 3}, 0);
}

TEST(BorderGeneric, GridFrameDimensions)
{
    const std::vector<std::vector<int>> g{{1, 2}, {3, 4}};
    const auto framed = mt::border(g, 0);
    EXPECT_EQ(framed.size(), 4u);
    EXPECT_EQ(framed.front(), (std::vector<int>{0, 0, 0, 0}));
    EXPECT_EQ(framed[1], (std::vector<int>{0, 1, 2, 0}));
}

TEST(MergeSetOpsGeneric, SortedMergeAndSetLaws)
{
    const std::vector<int> a{1, 3};
    const std::vector<int> b{2, 4};
    mt::test::expect_merge_sorted_union(a, b);
    EXPECT_EQ(mt::merge(a, b), (std::vector<int>{1, 2, 3, 4}));
    mt::test::expect_set_union_sorted(std::vector<int>{1, 2}, std::vector<int>{2, 3});
    mt::test::expect_set_intersection_sorted(std::vector<int>{1, 2, 3}, std::vector<int>{2, 3, 4});
}

TEST(CombinationGeneric, WalksNCkWithSortedSides)
{
    std::vector<int> v{1, 2, 3, 4};
    mt::test::expect_combination_walk(v, 2, 6);
}

TEST(MatchesGeneric, KmpOffsetsAndOverlaps)
{
    mt::test::expect_matches_at_valid_offsets("ababcababa", "aba", {0, 5, 7});
    mt::test::expect_matches_at_valid_offsets("aaaa", "aa", {0, 1, 2});
    EXPECT_TRUE(mt::matches("abc", "").empty());
    EXPECT_TRUE(mt::matches("ab", "abc").empty());
}

TEST(TextGeneric, LinesWordsRoundTrip)
{
    EXPECT_EQ(mt::lines("a\nb\n"), (std::vector<std::string>{"a", "b", ""}));
    EXPECT_EQ(mt::unlines(std::vector<std::string>{"a", "b"}), "a\nb");
    EXPECT_EQ(mt::words("  one  two three "), (std::vector<std::string>{"one", "two", "three"}));
    EXPECT_EQ(mt::unwords(std::vector<std::string>{"one", "two"}), "one two");
}

template <typename Container>
class CopyNWithBufferGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(CopyNWithBufferGeneric, mt::test::EopCoreContainers);

TYPED_TEST(CopyNWithBufferGeneric, PreservesSourceViaBuffer)
{
    constexpr int fill = 5;
    auto src = mt::test::make_iota_container<TypeParam>(fill);
    std::vector<int> dst(static_cast<std::size_t>(fill), 100);
    const auto last = mt::test::filled_end(src, fill);
    const auto src_copy = mt::test::to_filled_vector(src, fill);
    int buf[5]{};
    const int n = 3;
    (void)mt::copy_n_with_buffer(src.begin(), last, n, buf, dst.begin());
    mt::test::expect_ranges_equal(mt::test::to_filled_vector(src, fill), src_copy);
    EXPECT_EQ(dst[0], 0);
    EXPECT_EQ(dst[2], 2);
    EXPECT_EQ(dst[3], 100);
}

TEST(FindAllGeneric, CollectsEveryMatch)
{
    std::vector<int> v{1, 2, 1, 3, 1};
    const auto hits = mt::find_all(v, 1);
    ASSERT_EQ(hits.size(), 3u);
    for (const auto it : hits) {
        EXPECT_EQ(*it, 1);
    }
}

TEST(ReverseUntilGeneric, SwapsFromOutsideIn)
{
    std::vector<int> v{1, 2, 3, 4, 5};
    const auto [a, b] = mt::reverse_until(v.begin(), v.begin() + 2, v.end());
    EXPECT_EQ(v, (std::vector<int>{5, 4, 3, 2, 1}));
    EXPECT_EQ(a, v.begin() + 2);
    EXPECT_EQ(b, v.begin() + 3);
}

TEST(SortExtrasGeneric, SortSumsAndMedianSort)
{
    const std::vector<int> a{1, 3};
    const std::vector<int> b{0, 10};
    EXPECT_EQ(mt::sort_sums(a, b), (std::vector<int>{1, 3, 11, 13}));

    std::vector<int> v{9, 1, 8, 2, 7};
    mt::median_sort(v);
    mt::test::expect_sorted_prefix(v.begin(), v.end());
    EXPECT_EQ(v[2], 7);
}

}  // namespace
