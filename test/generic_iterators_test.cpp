#include "stlpp.h"

#include "support/eop_postconditions.h"
#include "support/fixtures.h"

#include <algorithm>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(NumericIteratorGeneric, RangeSizeAndDistance) {
    const auto r = mt::numeric_range(10, 15);
    EXPECT_EQ(r.size(), 5);
    EXPECT_EQ(*r.begin(), 10);
    EXPECT_EQ(r.end() - r.begin(), 5);
    mt::test::expect_ranges_equal(std::vector<int>(r.begin(), r.end()), mt::test::iota_vector(5, 10));
}

TEST(NumericIteratorGeneric, EmptyRange) {
    const auto r = mt::numeric_range(5, 5);
    EXPECT_EQ(r.size(), 0u);
    EXPECT_TRUE(r.begin() == r.end());
}

TEST(StepIteratorGeneric, EndDoesNotPassOnePastEnd) {
    std::vector<int> v{0, 1, 2, 3, 4};
    auto s = mt::step(v, 2);
    EXPECT_EQ(s.end() - s.begin(), 3);
    std::vector<int> out;
    std::copy(s.begin(), s.end(), std::back_inserter(out));
    EXPECT_EQ(out, (std::vector<int>{0, 2, 4}));
}

TEST(FilterIteratorGeneric, EndIncrementIsNoOp) {
    std::vector<int> v{1, 2, 3, 4};
    auto range = mt::filter(v, [](int x) { return x % 2 == 0; });
    auto e = range.end();
    ++e;
    EXPECT_TRUE(e == range.end());
}

TEST(ZipIteratorGeneric, DistinctPositionsNotEqual) {
    std::vector<int> a{1, 2};
    std::vector<int> b{10, 20};
    mt::zip_iterator it{a.begin(), b.begin()};
    mt::zip_iterator it2{a.begin(), b.begin() + 1};
    EXPECT_FALSE(it == it2);
}

TEST(CycleIteratorGeneric, RepeatsNTimesAndEndSafe) {
    std::vector<int> v{1, 2};
    auto cyc = mt::cycle(v, 2);
    std::vector<int> out;
    for (int x : cyc) {
        out.push_back(x);
    }
    EXPECT_EQ(out, (std::vector<int>{1, 2, 1, 2}));
    auto e = cyc.end();
    ++e;
    EXPECT_TRUE(e == cyc.end());
}

TEST(WindowIteratorGeneric, WindowCountAndEndSafe) {
    std::vector<int> v{1, 2, 3, 4};
    auto wr = mt::windows(v, 2);
    int count = 0;
    for (auto w : wr) {
        EXPECT_EQ(std::distance(w.begin(), w.end()), 2);
        ++count;
    }
    EXPECT_EQ(count, 3);
    auto we = wr.end();
    ++we;
    EXPECT_TRUE(we == wr.end());
}

TEST(UnfoldIteratorGeneric, EndIncrementAndDeref) {
    auto step = [](int n) -> std::optional<std::pair<int, int>> {
        if (n <= 0) {
            return std::nullopt;
        }
        return std::pair{n, n - 1};
    };
    using It = mt::unfold_iterator<decltype(step), int>;
    It end{};
    ++end;
    EXPECT_THROW((void)*end, std::bad_optional_access);
}

TEST(FlattenIteratorGeneric, FlattensOneLevel) {
    std::vector<std::vector<int>> nested{{1, 2}, {}, {3}};
    std::vector<int> out;
    for (int x : mt::flattened(nested)) {
        out.push_back(x);
    }
    mt::test::expect_ranges_equal(out, std::vector<int>{1, 2, 3});
}

TEST(FlattenIteratorGeneric, EndIncrementIsNoOp) {
    std::vector<std::vector<int>> nested{{1}};
    auto range = mt::flattened(nested);
    auto e = range.end();
    ++e;
    EXPECT_TRUE(e == range.end());
}

TEST(ChunksIteratorGeneric, PartitionCoverAndEndSafe) {
    std::vector<int> v{1, 2, 3, 4, 5};
    std::vector<std::vector<int>> chunks;
    for (auto c : mt::chunks(v, 2)) {
        chunks.push_back({c.begin(), c.end()});
    }
    EXPECT_EQ(chunks, (std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5}}));
    EXPECT_TRUE(mt::chunks(v, 0).begin() == mt::chunks(v, 0).end());
    auto empty = mt::chunks(v, 2);
    auto e = empty.end();
    ++e;
    EXPECT_TRUE(e == empty.end());
}

TEST(EdgeIteratorGeneric, ConsecutivePairsCount) {
    std::vector<int> v{1, 2, 3, 4};
    std::vector<std::pair<int, int>> out;
    for (auto e : mt::edges(v)) {
        out.push_back(e);
    }
    EXPECT_EQ(out, (std::vector<std::pair<int, int>>{{1, 2}, {2, 3}, {3, 4}}));
    std::vector<int> single{1};
    EXPECT_TRUE(mt::edges(single).begin() == mt::edges(single).end());
}

TEST(TransformIteratorGeneric, AppliesFunctionOnDeref) {
    std::vector<int> v{1, 2, 3};
    auto t = mt::transform(v, [](int x) { return x + 10; });
    std::vector<int> out;
    for (int x : t) {
        out.push_back(x);
    }
    mt::test::expect_ranges_equal(out, (std::vector<int>{11, 12, 13}));
}

TEST(CountingOutputIteratorGeneric, CountsAssignments) {
    std::vector<int> v{1, 2, 3, 4};
    mt::counting_output_iterator out;
    std::copy(v.begin(), v.end(), out);
    EXPECT_EQ(out.count(), 4u);
}

}  // namespace
