#include "stlpp.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <list>
#include <numeric>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(Integer, OddHalfHighByte_Examples) {
    EXPECT_TRUE(mt::odd(3));
    EXPECT_FALSE(mt::odd(4));
    EXPECT_EQ(mt::half(6), 3);
    EXPECT_EQ(mt::half(7), 3);
    EXPECT_EQ(mt::high_byte(std::uint16_t{0x1234}), 0x12);
}

TEST(IteratorOps, SuccessorPredecessorAdvance) {
    std::vector<int> v{10, 20, 30, 40};
    auto it = v.begin();
    mt::advance(it, 2);
    EXPECT_EQ(*it, 30);
    EXPECT_EQ(*mt::successor(v.begin()), 20);
    EXPECT_EQ(*mt::predecessor(v.end()), 40);
}

TEST(Apply, SquaresEachElement) {
    std::vector<int> v{1, 2, 3};
    mt::apply(v, [](int x) { return x * x; });
    EXPECT_EQ(v, (std::vector<int>{1, 4, 9}));
}

TEST(ApplyUntil, StopsBeforeMatchingElement) {
    std::vector<int> v{1, 2, 3, 4};
    auto stop = mt::apply_until(v, [](int x) { return x + 10; }, [](int x) { return x == 3; });
    EXPECT_EQ(v, (std::vector<int>{11, 12, 3, 4}));
    EXPECT_EQ(*stop, 3);
}

TEST(FindIfN, ReturnsIteratorAndRemaining) {
    std::vector<int> v{1, 3, 4, 6};
    auto [it, left] = mt::find_if_n(v.begin(), 4, [](int x) { return x % 2 == 0; });
    EXPECT_EQ(*it, 4);
    EXPECT_EQ(left, 2);
}

TEST(FindBackward, LastMatchOrEnd) {
    std::vector<int> v{5, 3, 0, 1, 0, 8};
    auto p = mt::find_backward(v.begin(), v.end(), 0);
    EXPECT_EQ(p, v.begin() + 4);
    EXPECT_EQ(mt::find_backward(v.begin(), v.end(), 9), v.end());
}

TEST(FindBackwardIf, LastPredicateMatch) {
    std::vector<int> v{5, 3, 0, 1, 8};
    auto p = mt::find_backward_if(v.begin(), v.end(), [](int x) { return x == 0; });
    EXPECT_EQ(*p, 0);
    EXPECT_EQ(mt::find_backward_if(v.begin(), v.end(), [](int) { return false; }), v.end());
}

TEST(FindBackwardIfN, OnlyExaminesLastN) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto p = mt::find_backward_if_n(v.begin(), v.end(), 2, [](int x) { return x % 2 == 0; });
    EXPECT_EQ(*p, 4);
}

TEST(FindBackwardIfNot, LastNonMatch) {
    std::vector<int> v{2, 4, 5, 8};
    auto p = mt::find_backward_if_not(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
    EXPECT_EQ(*p, 5);
}

TEST(FindAll, CollectsEveryMatch) {
    std::vector<int> v{1, 2, 1, 3};
    auto hits = mt::find_all(v, 1);
    ASSERT_EQ(hits.size(), 2u);
    EXPECT_EQ(*hits[0], 1);
    EXPECT_EQ(*hits[1], 1);
}

TEST(RemoveIfNot, KeepsPredicateMatches) {
    std::vector<int> v{9, 0, 1, 4, 7};
    v.erase(mt::remove_if_not(v, [](int x) { return x < 5; }), v.end());
    EXPECT_EQ(v, (std::vector<int>{0, 1, 4}));
}

TEST(RemoveIfNotBackward, KeepsSuffixOfMatches) {
    std::vector<int> v{9, 0, 1, 4, 7};
    auto w = mt::remove_if_not_backward(v.begin(), v.end(), [](int x) { return x < 5; });
    EXPECT_EQ(std::vector<int>(w, v.end()), (std::vector<int>{0, 1, 4}));
}

TEST(ReverseN, ReversesForwardIteratorRange) {
    std::list<int> xs{1, 2, 3, 4};
    mt::reverse_n(xs.begin(), 4);
    EXPECT_EQ(xs, (std::list<int>{4, 3, 2, 1}));
}

TEST(ReverseUntil, SwapsFromOutsideIn) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto [a, b] = mt::reverse_until(v.begin(), v.begin() + 2, v.end());
    EXPECT_EQ(v.front(), 5);
    EXPECT_EQ(a, v.begin() + 2);
    EXPECT_EQ(b, v.begin() + 3);
}

TEST(ReverseNWithBuffer, ReversesThroughBuffer) {
    std::list<int> xs{1, 2, 3, 4};
    int buf[4]{};
    mt::reverse_n_with_buffer(xs.begin(), 4, buf);
    EXPECT_EQ(xs, (std::list<int>{4, 3, 2, 1}));
}

TEST(CopyN, CopiesCountedPrefix) {
    std::vector<int> src{1, 2, 3, 4};
    std::vector<int> dst(2, 0);
    auto [in, out] = mt::copy_n(src.begin(), 2, dst.begin());
    EXPECT_EQ(dst, (std::vector<int>{1, 2}));
    EXPECT_EQ(*in, 3);
    EXPECT_EQ(out, dst.end());
}

TEST(CopyNWithBuffer, CopiesViaBuffer) {
    std::vector<int> src{1, 2, 3};
    int buf[3]{};
    std::vector<int> dst(3, 0);
    auto [in, out] = mt::copy_n_with_buffer(src.begin(), 3, buf, dst.begin());
    EXPECT_EQ(dst, src);
    EXPECT_EQ(in, src.end());
    EXPECT_EQ(out, dst.end());
}

TEST(SwapRangesN, SwapsPrefix) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{7, 8, 9};
    auto [ia, ib] = mt::swap_ranges_n(a.begin(), b.begin(), 2);
    EXPECT_EQ(a, (std::vector<int>{7, 8, 3}));
    EXPECT_EQ(b, (std::vector<int>{1, 2, 9}));
    EXPECT_EQ(*ia, 3);
    EXPECT_EQ(*ib, 9);
}

TEST(Slide, MovesWindow) {
    std::vector<int> v{0, 1, 2, 3, 4, 5};
    auto [lo, hi] = mt::slide(v.begin() + 1, v.begin() + 3, v.begin() + 5);
    EXPECT_EQ(std::vector<int>(lo, hi), (std::vector<int>{1, 2}));
}

TEST(SlideGather, READMEGatherExample) {
    std::array<int, 10> a{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
    auto pos = a.begin() + 4;
    auto [lo, hi] = mt::gather(a.begin(), a.end(), pos, [](int x) { return x < 2; });
    EXPECT_TRUE(std::all_of(lo, hi, [](int x) { return x < 2; }));
}

TEST(PartitionPointN, FirstFalseInPartition) {
    std::vector<int> v{1, 2, 3, 8, 9};
    auto p = mt::partition_point_n(v.begin(), 5, [](int x) { return x < 5; });
    EXPECT_EQ(*p, 8);
}

TEST(Sort, BinaryInsertionAndSubrange) {
    std::vector<int> v{3, 1, 4, 1};
    mt::binary_insertion_sort_n(v.begin(), 4);
    EXPECT_EQ(v, (std::vector<int>{1, 1, 3, 4}));

    std::vector<int> w{9, 1, 8, 2, 7, 3};
    mt::sort_subrange(w.begin(), w.end(), w.begin() + 1, w.begin() + 4);
    EXPECT_TRUE(std::is_sorted(w.begin() + 1, w.begin() + 4));
}

TEST(SortSums, PairwiseSumsSorted) {
    std::vector<int> a{1, 3};
    std::vector<int> b{0, 10};
    EXPECT_EQ(mt::sort_sums(a, b), (std::vector<int>{1, 3, 11, 13}));
}

TEST(MedianSort, SortsWholeRange) {
    std::vector<int> v{9, 1, 8, 2, 7, 3, 6, 4, 5, 0, 11, 10, 12, 14, 13, 15, 16};
    mt::median_sort(v);
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST(Slice, ClipsToBounds) {
    std::vector<int> v{0, 1, 2, 3, 4};
    auto [a, b] = mt::slice(v.begin(), v.end(), 1, 3);
    EXPECT_EQ(std::vector<int>(a, b), (std::vector<int>{1, 2, 3}));
    auto empty = mt::slice(v, 10, 3);
    EXPECT_TRUE(empty.empty());
}

TEST(Gcd, SteinAndExtended) {
    EXPECT_EQ(mt::stein_gcd(48, 18), 6);
    EXPECT_EQ(mt::stein_gcd(0, 7), 7);
    auto [g, x, y] = mt::extended_gcd(30, 12);
    EXPECT_EQ(g, 6);
    EXPECT_EQ(30 * x + 12 * y, 6);
}

TEST(MinMax, FindsExtremes) {
    std::vector<int> v{3, 1, 4, 1, 5};
    auto [lo, hi] = mt::min_max(v.begin(), v.end());
    EXPECT_EQ(*lo, 1);
    EXPECT_EQ(*hi, 5);
}

TEST(SkipTake, PrefixAndDrop) {
    std::vector<int> v{1, 2, 3, 4, 5};
    auto skipped = mt::skip(v, 2);
    EXPECT_EQ(skipped.front(), 3);
    auto taken = mt::take(v, 3);
    EXPECT_EQ(std::vector<int>(taken.begin(), taken.end()), (std::vector<int>{1, 2, 3}));
}

TEST(SkipWhileTakeWhile, PredicateBounds) {
    std::vector<int> v{-1, -2, 3, 4};
    auto skipped = mt::skip_while(v, [](int x) { return x < 0; });
    EXPECT_EQ(skipped.front(), 3);
    auto taken = mt::take_while(v, [](int x) { return x < 0; });
    EXPECT_EQ(std::vector<int>(taken.begin(), taken.end()), (std::vector<int>{-1, -2}));
}

TEST(IotaHillValley, FillsShapes) {
    std::vector<int> v(5);
    mt::iota(v, 10);
    EXPECT_EQ(v, (std::vector<int>{10, 11, 12, 13, 14}));
    mt::hill(v.begin(), v.end());
    EXPECT_EQ(v, (std::vector<int>{0, 1, 2, 1, 0}));
    mt::valley(v.begin(), v.end());
    EXPECT_EQ(v, (std::vector<int>{1, 0, 0, 1, 2}));
    mt::reverse_iota(v.begin(), v.end());
    EXPECT_EQ(v, (std::vector<int>{4, 3, 2, 1, 0}));
}

TEST(ZipUnzip, PairsAndSplits) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{10, 20, 30};
    auto z = mt::zip(a, b);
    ASSERT_EQ(z.size(), 3u);
    EXPECT_EQ(z[1], (std::pair<int, int>{2, 20}));

    std::vector<int> out_a(3), out_b(3);
    auto [oa, ob] = mt::unzip(z.begin(), z.end(), out_a.begin(), out_b.begin());
    (void)oa;
    (void)ob;
    EXPECT_EQ(out_a, a);
    EXPECT_EQ(out_b, b);
}

TEST(ZipWithZipN, CombinesAndCounts) {
    std::vector<int> a{1, 2};
    std::vector<int> b{10, 20};
    std::vector<int> summed;
    mt::zip_with(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(summed), std::plus<int>{});
    EXPECT_EQ(summed, (std::vector<int>{11, 22}));

    std::vector<std::pair<int, int>> pairs;
    auto written = mt::zip_n(a.begin(), b.begin(), 2, std::back_inserter(pairs));
    (void)written;
    EXPECT_EQ(pairs.front(), (std::pair<int, int>{1, 10}));
}

TEST(StartsEndsWith, PrefixAndSuffix) {
    EXPECT_TRUE(mt::starts_with(std::string{"https://ex.com"}, std::string{"https://"}));
    EXPECT_TRUE(mt::ends_with(std::string{"file.cpp"}, std::string{".cpp"}));
    EXPECT_TRUE(mt::end_with(std::string{"file.cpp"}, std::string{".cpp"}));
    EXPECT_FALSE(mt::starts_with(std::string{"abc"}, std::string{"abcd"}));
}

TEST(UpperBoundN, FirstGreater) {
    std::vector<int> v{1, 2, 2, 4};
    auto u = mt::upper_bound_n(v.begin(), 4, 2);
    EXPECT_EQ(*u, 4);
}

TEST(StablePartitionPosition, READMEMask) {
    int a[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};
    bool mask[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0};
    auto mid = mt::stable_partition_position(std::begin(a), std::end(a), [&](int* it) {
        return mask[it - std::begin(a)];
    });
    EXPECT_EQ(std::vector<int>(std::begin(a), mid), (std::vector<int>{2, 4, 4, 2}));
    EXPECT_EQ(std::vector<int>(mid, std::end(a)), (std::vector<int>{1, 3, 5, 5, 3, 1}));
}

TEST(Pairwise, ConsecutivePairs) {
    EXPECT_EQ(mt::pairwise(std::vector<int>{1, 2, 3, 4}),
              (std::vector<std::pair<int, int>>{{1, 2}, {2, 3}, {3, 4}}));
    EXPECT_TRUE(mt::pairwise(std::vector<int>{1}).empty());
}

TEST(Concat, AppendsRanges) {
    EXPECT_EQ(mt::concat(std::vector<int>{1, 2}, std::vector<int>{3}), (std::vector<int>{1, 2, 3}));
}

TEST(Intercalate, InsertsSeparator) {
    std::vector<char> letters{'a', 'b', 'c'};
    std::string s;
    mt::intercalate(',', letters.begin(), letters.end(), std::back_inserter(s));
    EXPECT_EQ(s, "a,b,c");
}

TEST(Group, ConsecutiveRuns) {
    EXPECT_EQ(mt::group(std::vector<int>{1, 1, 2, 2, 2, 1}),
              (std::vector<std::vector<int>>{{1, 1}, {2, 2, 2}, {1}}));
}

TEST(ShiftBy, RotatesLeftAndRight) {
    std::vector<int> v{1, 2, 3, 4, 5};
    mt::shift_by(v, 2);
    EXPECT_EQ(v, (std::vector<int>{3, 4, 5, 1, 2}));
    mt::shift_by(v, -2);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(Transpose, SwapsRowsAndColumns) {
    std::vector<std::vector<int>> m{{1, 2, 3}, {4, 5, 6}};
    EXPECT_EQ(mt::transpose(m), (std::vector<std::vector<int>>{{1, 4}, {2, 5}, {3, 6}}));
}

TEST(Border, FramesOneAndTwoD) {
    EXPECT_EQ(mt::border(std::vector<int>{1, 2, 3}, 0), (std::vector<int>{0, 1, 2, 3, 0}));
    std::vector<std::vector<int>> g{{1, 2}, {3, 4}};
    auto framed = mt::border(g, 0);
    EXPECT_EQ(framed.size(), 4u);
    EXPECT_EQ(framed.front(), (std::vector<int>{0, 0, 0, 0}));
    EXPECT_EQ(framed[1], (std::vector<int>{0, 1, 2, 0}));
}

TEST(Text, LinesWordsRoundTrip) {
    EXPECT_EQ(mt::lines("a\nb\n"), (std::vector<std::string>{"a", "b", ""}));
    EXPECT_EQ(mt::unlines(std::vector<std::string>{"a", "b"}), "a\nb");
    EXPECT_EQ(mt::words("  one  two three "), (std::vector<std::string>{"one", "two", "three"}));
    EXPECT_EQ(mt::unwords(std::vector<std::string>{"one", "two"}), "one two");
}

TEST(Matches, KmpOverlappingHits) {
    EXPECT_EQ(mt::matches("ababcababa", "aba"), (std::vector<std::size_t>{0, 5, 7}));
    EXPECT_EQ(mt::matches("aaaa", "aa"), (std::vector<std::size_t>{0, 1, 2}));
    EXPECT_TRUE(mt::matches("abc", "").empty());
}

TEST(NextCombination, WalksC42) {
    std::vector<int> v{1, 2, 3, 4};
    int count = 0;
    do {
        ++count;
        EXPECT_TRUE(std::is_sorted(v.begin(), v.begin() + 2));
        EXPECT_TRUE(std::is_sorted(v.begin() + 2, v.end()));
    } while (mt::next_combination(v.begin(), v.begin() + 2, v.end()));
    EXPECT_EQ(count, 6);
}
