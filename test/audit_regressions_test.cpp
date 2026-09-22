#include "stlpp.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(AuditRegression, ExclusiveScanSupportsInPlaceOutput)
{
    std::vector<int> values{1, 2, 3, 4};
    EXPECT_EQ(mt::exclusive_scan(values.begin(), values.end(), values.begin(), 0, std::plus<>{}),
              values.end());
    EXPECT_EQ(values, (std::vector<int>{0, 1, 3, 6}));
}

TEST(AuditRegression, BalancedReductionPreservesNoncommutativeOrder)
{
    std::vector<std::string> values;
    std::string expected;
    for (int i = 0; i < 40; ++i) {
        values.push_back(std::to_string(i));
        expected += values.back();
        EXPECT_EQ(mt::reduce_balanced(values, std::plus<>{}, std::string{}), expected);
    }
}

TEST(AuditRegression, ProxyAccumulatorsDoNotMutateInput)
{
    std::vector<bool> values{true, false, true};
    const auto original = values;
    EXPECT_EQ(mt::scan(values, std::logical_and<>{}), (std::vector<bool>{true, false, false}));
    EXPECT_FALSE(mt::reduce_nonempty(values, std::logical_and<>{}));
    EXPECT_EQ(values, original);
}

TEST(AuditRegression, BufferedPartitionReturnsBoundary)
{
    for (const std::vector<int>& input : {std::vector<int>{}, {1, 3}, {2, 4}, {1, 2, 3, 4}}) {
        auto actual = input;
        auto expected = input;
        std::vector<int> buffer(input.size());
        auto even = [](int x) { return x % 2 == 0; };
        auto boundary = std::stable_partition(expected.begin(), expected.end(), even);
        auto result =
            mt::partition_stable_with_buffer(actual.begin(), actual.end(), buffer.begin(), even);
        EXPECT_EQ(actual, expected);
        EXPECT_EQ(result - actual.begin(), boundary - expected.begin());
    }
}

TEST(AuditRegression, BackwardMergePreservesEquivalentElementOrder)
{
    using Item = std::pair<int, int>;
    std::vector<Item> left{{1, 0}, {2, 1}}, right{{1, 2}, {2, 3}}, actual(4), expected(4);
    auto comp = [](const Item& a, const Item& b) { return a.first < b.first; };
    std::merge(left.begin(), left.end(), right.begin(), right.end(), expected.begin(), comp);
    EXPECT_EQ(mt::merge_copy_backward(left.begin(), left.end(), right.begin(), right.end(),
                                      actual.end(), comp),
              actual.begin());
    EXPECT_EQ(actual, expected);
}

TEST(AuditRegression, CountedBoundsDoNotNarrowRangeLength)
{
    std::vector<int> values(300);
    EXPECT_EQ(mt::bounded_count(values.begin(), values.end(), std::uint8_t{100}), 100);
    EXPECT_EQ(mt::bounded_count(values.begin(), values.end(), std::int8_t{100}), 100);
}

TEST(AuditRegression, LargeStridesDoNotOverflow)
{
    std::array values{1, 2, 3};
    auto range = mt::step(values, std::numeric_limits<std::ptrdiff_t>::max());
    EXPECT_EQ(range.end() - range.begin(), 1);
    EXPECT_EQ(*range.begin(), 1);
    EXPECT_EQ(range.end().base(), values.end());
    auto huge = mt::step(
        mt::numeric_iterator<std::ptrdiff_t>{0},
        mt::numeric_iterator<std::ptrdiff_t>{std::numeric_limits<std::ptrdiff_t>::max()}, 2);
    EXPECT_EQ(huge.end().base(),
              mt::numeric_iterator<std::ptrdiff_t>{std::numeric_limits<std::ptrdiff_t>::max()});
}

TEST(AuditRegression, TreeCopiesOwnTheirLinksAndMovesEmptyTheSource)
{
    mt::btree<int> original;
    auto root = original.set_root(1);
    original.add_left(root, 2);
    original.add_right(root, 3);
    mt::btree<int> copy = original;
    mt::sink(copy.root()) = 10;
    mt::sink(mt::left_successor(copy.root())) = 20;
    EXPECT_EQ(mt::source(original.root()), 1);
    EXPECT_EQ(mt::source(mt::left_successor(original.root())), 2);
    EXPECT_EQ(mt::predecessor(mt::left_successor(copy.root())), copy.root());
    mt::btree<int> assigned;
    assigned = copy;
    copy = mt::btree<int>{};
    EXPECT_EQ(mt::source(mt::right_successor(assigned.root())), 3);
    auto moved = std::move(assigned);
    EXPECT_EQ(moved.size(), 3u);
    EXPECT_EQ(assigned.size(), 0u);
    EXPECT_TRUE(mt::empty(assigned.root()));
    assigned.set_root(9);
    EXPECT_EQ(mt::source(moved.root()), 10);
}

TEST(AuditRegression, HashTableGrowsWithMoveOnlyValues)
{
    mt::htable<int, std::unique_ptr<int>> table;
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(table.insert(i, std::make_unique<int>(i)));
    }
    for (int i = 0; i < 100; ++i) {
        ASSERT_NE(table.find(i), nullptr);
        EXPECT_EQ(**table.find(i), i);
    }
    auto moved = std::move(table);
    EXPECT_TRUE(table.empty());
    EXPECT_TRUE(table.insert(5, std::make_unique<int>(42)));
    EXPECT_EQ(table.size(), 1u);
    EXPECT_EQ(moved.size(), 100u);
}

struct ThrowingCopy {
    static inline bool fail = false;
    int value;
    explicit ThrowingCopy(int v) : value(v) {}
    ThrowingCopy(const ThrowingCopy& other) : value(other.value)
    {
        if (fail)
            throw std::runtime_error("copy");
    }
    ThrowingCopy(ThrowingCopy&&) = default;
    ThrowingCopy& operator=(const ThrowingCopy&) = default;
    ThrowingCopy& operator=(ThrowingCopy&&) = default;
};

TEST(AuditRegression, HashTableSurvivesFailedRehash)
{
    mt::htable<int, ThrowingCopy> table;
    for (int i = 0; i < 6; ++i)
        table.insert(i, ThrowingCopy{i});
    ThrowingCopy::fail = true;
    EXPECT_THROW(table.insert(6, ThrowingCopy{6}), std::runtime_error);
    ThrowingCopy::fail = false;
    EXPECT_EQ(table.size(), 6u);
    for (int i = 0; i < 6; ++i) {
        ASSERT_NE(table.find(i), nullptr);
        EXPECT_EQ(table.find(i)->value, i);
    }
    EXPECT_TRUE(table.insert(6, ThrowingCopy{6}));
}

TEST(AuditRegression, MovedRingBufferIsSafelyEmpty)
{
    mt::ring_buffer<int> source(3);
    source.try_push(1);
    source.try_push(2);
    (void)source.try_pop();
    auto destination = std::move(source);
    EXPECT_TRUE(source.empty());
    EXPECT_FALSE(source.try_pop());
    EXPECT_THROW((void)source[0], std::out_of_range);
    EXPECT_EQ(destination.try_pop(), 2);
}

TEST(AuditRegression, QuadtreeTerminatesAtFloatingPointResolution)
{
    const double next = std::nextafter(1.0, 2.0);
    mt::quadtree tree({1, 1, next, next});
    for (int i = 0; i < 20; ++i)
        EXPECT_TRUE(tree.insert({1, 1}));
    EXPECT_EQ(tree.query({1, 1, next, next}).size(), 20u);
    mt::quadtree large({1e308, 1e308, 1.7e308, 1.7e308});
    for (int i = 0; i < 20; ++i)
        EXPECT_TRUE(large.insert({1.2e308, 1.2e308}));
    EXPECT_EQ(large.query({1e308, 1e308, 1.7e308, 1.7e308}).size(), 20u);
}

TEST(AuditRegression, FlattenSupportsConstAndTemporaryInnerRanges)
{
    const std::vector<std::vector<int>> input{{1, 2}, {}, {3}};
    auto const_range = mt::flattened(input);
    std::vector<int> collected;
    for (int x : const_range)
        collected.push_back(x);
    EXPECT_EQ(collected, (std::vector<int>{1, 2, 3}));
    auto temporary = std::views::iota(0, 3) |
                     std::views::transform([](int n) { return std::vector{n, n + 10}; });
    auto flat = mt::flattened(temporary);
    auto saved = [&] {
        auto original = flat.begin();
        return original++;
    }();
    EXPECT_EQ(*saved, 0);
    ++saved;
    EXPECT_EQ(*saved, 10);
    ++saved;
    EXPECT_EQ(*saved, 1);
}

TEST(AuditRegression, AlgorithmsAcceptDistinctSentinels)
{
    int input[]{1, 2, 3, 4};
    auto range = std::ranges::subrange(std::counted_iterator(input, 4), std::default_sentinel);
    EXPECT_TRUE(mt::all(range, [](int n) { return n > 0; }));
    EXPECT_TRUE(mt::any(range, [](int n) { return n == 3; }));
    EXPECT_TRUE(mt::none(range, [](int n) { return n == 0; }));
    EXPECT_EQ(mt::count_if(range, [](int n) { return n % 2 == 0; }), 2);
    EXPECT_EQ(*mt::span_if(range, [](int n) { return n < 3; }), 3);
    EXPECT_EQ(*mt::break_if(range, [](int n) { return n == 3; }), 3);
    EXPECT_EQ(mt::map(range, [](int n) { return n * 2; }), (std::vector<int>{2, 4, 6, 8}));
    EXPECT_EQ(mt::reduce(range, 0, std::plus<>{}), 10);
    EXPECT_EQ(*mt::pfind(range, 3), 3);
    EXPECT_EQ(mt::pfind(range, 0), range.end());
    EXPECT_EQ(mt::merge(range, range), (std::vector<int>{1, 1, 2, 2, 3, 3, 4, 4}));
    EXPECT_EQ(mt::set_union(range, range), (std::vector<int>{1, 2, 3, 4}));
    EXPECT_EQ(mt::set_intersection(range, range), (std::vector<int>{1, 2, 3, 4}));
    EXPECT_EQ(mt::adjacent_difference(range), (std::vector<int>{1, 1, 1, 1}));
    EXPECT_EQ(mt::partition_copy(range, [](int n) { return n % 2 == 0; }).first,
              (std::vector<int>{2, 4}));
    EXPECT_EQ(mt::equal_range(range.begin(), range.end(), 3).first.base(), input + 2);
    EXPECT_EQ(mt::sort_sums(range, range).size(), 16u);
    EXPECT_EQ(mt::cartesian_product(range, range).size(), 16u);
    EXPECT_EQ(mt::mapcat(range, [&](int) { return range; }).size(), 16u);
}

struct NoDefault {
    int value;
    explicit NoDefault(int v) : value(v) {}
};

TEST(AuditRegression, ParallelResultsNeedNotBeDefaultConstructible)
{
    std::vector<int> input(5000, 1);
    auto mapped = mt::map(input, [](int v) { return NoDefault{v}; });
    EXPECT_EQ(mapped.size(), input.size());
    auto reduced = mt::reduce(mapped, NoDefault{0}, [](NoDefault a, NoDefault b) {
        return NoDefault{a.value + b.value};
    });
    EXPECT_EQ(reduced.value, 5000);
    auto flags = mt::map(input, [](int n) { return n == 1; });
    EXPECT_EQ(flags, std::vector<bool>(5000, true));
    std::fill(flags.begin(), flags.end(), false);
    mt::map(input.begin(), input.end(), flags.begin(), [](int n) { return n == 1; });
    EXPECT_EQ(flags, std::vector<bool>(5000, true));
}

TEST(AuditRegression, BucketSortHandlesExtremeFiniteValues)
{
    std::vector<double> input(5000);
    for (std::size_t i = 0; i < input.size(); ++i)
        input[i] = i % 2 ? 1e308 : -1e308;
    auto expected = input;
    std::sort(expected.begin(), expected.end());
    mt::partial_bucketsort(input);
    EXPECT_EQ(input, expected);
}

TEST(AuditRegression, NonpositiveCountsAreNoOps)
{
    std::array values{3, 1, 2};
    std::array<int, 3> buffer{};
    EXPECT_EQ(mt::sort_n_with_buffer(values.begin(), -3, buffer.begin()), values.begin());
    EXPECT_EQ(mt::sort_n_adaptive(values.begin(), -3, buffer.begin(), 3), values.begin());
    EXPECT_EQ(mt::power_unary(42, -1, [](int n) { return n + 1; }), 42);
    EXPECT_EQ(values, (std::array{3, 1, 2}));
}

TEST(AuditRegression, ModularMultiplicationHandlesMinimumSignedModulus)
{
    constexpr auto min = std::numeric_limits<long long>::min();
    mt::modulo_multiply<long long> multiply(min);
    EXPECT_EQ(multiply(-1, 1), std::numeric_limits<long long>::max());
    EXPECT_EQ(multiply(min, -1), 0);
    EXPECT_EQ(multiply(-2, -3), 6);
}

TEST(AuditRegression, IotaDoesNotIncrementPastTheLastValue)
{
    std::array<int, 2> values{};
    const auto max = std::numeric_limits<int>::max();
    mt::iota(values, max - 1);
    EXPECT_EQ(values, (std::array{max - 1, max}));
}

TEST(AuditRegression, ProxySortingAndCyclesSaveValues)
{
    std::vector<bool> values{true, false, true, false};
    mt::binary_insertion_sort_n(values.begin(), values.size());
    EXPECT_EQ(values, (std::vector<bool>{false, false, true, true}));
    values = {true, false, false};
    mt::cycle_from(values.begin(),
                   [&](auto it) { return it + 1 == values.end() ? values.begin() : it + 1; });
    EXPECT_EQ(values, (std::vector<bool>{false, false, true}));
}

TEST(AuditRegression, GlobalReplacementSupportsAliasingViews)
{
    std::string value = "abcabcabcabcabcabcabcabcabcabc";
    std::string_view from(value.data(), 3);
    std::string_view to(value.data() + 1, 2);
    mt::gsub(value, from, to);
    EXPECT_EQ(value, "bcbcbcbcbcbcbcbcbcbc");
    EXPECT_FALSE(mt::vowel{}('\0'));
    EXPECT_TRUE(mt::vowel{}('A'));
}

TEST(AuditRegression, RotatingTraversalRestoresRootParent)
{
    mt::btree<int> tree;
    auto root = tree.set_root(1);
    auto left = tree.add_left(root, 2);
    tree.add_right(root, 3);
    tree.add_left(left, 4);
    EXPECT_EQ(mt::weight_rotating(root), 4u);
    EXPECT_FALSE(mt::has_predecessor(root));
    EXPECT_EQ(mt::weight_rotating(left), 2u);
    EXPECT_EQ(mt::predecessor(left), root);
    EXPECT_EQ(mt::weight(root), 4u);
}

TEST(AuditRegression, NumericDistanceAcrossSignedBoundary)
{
    const auto boundary = static_cast<std::uint64_t>(std::numeric_limits<std::ptrdiff_t>::max());
    mt::numeric_iterator<std::uint64_t> low{boundary}, high{boundary + 1};
    EXPECT_EQ(high - low, 1);
    EXPECT_EQ(low - high, -1);
    EXPECT_EQ(mt::numeric_range(boundary, boundary + 1).size(), 1);
    const auto min = std::numeric_limits<std::ptrdiff_t>::min();
    mt::numeric_iterator<std::ptrdiff_t> value{min};
    value -= min;
    EXPECT_EQ(*value, 0);
}

struct ThrowingMove {
    static inline bool fail = false;
    int value = 0;
    explicit ThrowingMove(int v = 0) : value(v) {}
    ThrowingMove(const ThrowingMove&) = default;
    ThrowingMove(ThrowingMove&& other) : value(other.value)
    {
        if (fail)
            throw std::runtime_error("move");
    }
    ThrowingMove& operator=(const ThrowingMove&) = default;
    ThrowingMove& operator=(ThrowingMove&&) = default;
    friend bool operator<(const ThrowingMove& a, const ThrowingMove& b)
    {
        return a.value < b.value;
    }
};

TEST(AuditRegression, PriorityQueueFailedPushDoesNotPublishAHandle)
{
    mt::dynamic_pqueue<ThrowingMove> queue;
    ThrowingMove::fail = true;
    EXPECT_THROW(queue.push(ThrowingMove{1}), std::runtime_error);
    ThrowingMove::fail = false;
    EXPECT_TRUE(queue.empty());
    EXPECT_FALSE(queue.contains(0));
    const auto handle = queue.push(ThrowingMove{2});
    queue.pop();
    ThrowingMove::fail = true;
    EXPECT_THROW(queue.push(ThrowingMove{3}), std::runtime_error);
    ThrowingMove::fail = false;
    EXPECT_FALSE(queue.contains(handle));
    EXPECT_TRUE(queue.empty());
    const auto reused = queue.push(ThrowingMove{4});
    EXPECT_TRUE(queue.contains(reused));
    EXPECT_EQ(queue.top().value, 4);
}

TEST(AuditRegression, ZeroPriorityQueuePrintWidthDoesNotDivideByZero)
{
    std::priority_queue<int> queue;
    queue.push(1);
    queue.push(2);
    std::ostringstream output;
    mt::print(queue, output, 0);
    EXPECT_EQ(output.str(), "2 1 \n");
}

TEST(AuditRegression, MapAcceptsMutableCallables)
{
    std::vector<int> input(5000, 3);
    const auto result = mt::map(input, [factor = 2](int n) mutable { return n * factor; });
    EXPECT_EQ(result, std::vector<int>(5000, 6));
}

}  // namespace
