#include "stlpp.h"

#include <functional>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

TEST(Power, MultiplyAndPlusAndIdentity) {
    EXPECT_EQ(mt::power(2, 10, std::multiplies<int>{}), 1024);
    EXPECT_EQ(mt::power(3, 5, std::plus<int>{}), 15);
    EXPECT_EQ(mt::power(2, 0, std::multiplies<int>{}, 1), 1);
    EXPECT_TRUE(mt::even(4));
    EXPECT_FALSE(mt::even(3));
}

TEST(Scan, InclusiveAndExclusive) {
    EXPECT_EQ(mt::scan(std::vector{1, 2, 3, 4}, std::plus<>{}), (std::vector<int>{1, 3, 6, 10}));
    EXPECT_EQ(mt::exclusive_scan(std::vector{1, 2, 3}, 0, std::plus<>{}),
              (std::vector<int>{0, 1, 3}));
    EXPECT_TRUE(mt::scan(std::vector<int>{}, std::plus<>{}).empty());
}

TEST(Unfold, CountdownAndIterate) {
    auto step = [](int n) -> std::optional<std::pair<int, int>> {
        if (n <= 0) {
            return std::nullopt;
        }
        return std::pair{n, n - 1};
    };
    EXPECT_EQ(mt::unfold(step, 3), (std::vector<int>{3, 2, 1}));
    EXPECT_EQ(mt::iterate_n(5, 1, [](int x) { return x * 2; }),
              (std::vector<int>{1, 2, 4, 8, 16}));
}

TEST(SpanSplit, PredAndCount) {
    std::vector<int> v{2, 4, 5, 6};
    EXPECT_EQ(mt::span_if(v, [](int x) { return x % 2 == 0; }), v.begin() + 2);
    EXPECT_EQ(mt::break_if(v, [](int x) { return x == 5; }), v.begin() + 2);
    EXPECT_EQ(mt::split_at(v.begin(), v.end(), 2), v.begin() + 2);
    EXPECT_EQ(mt::split_at(v.begin(), v.end(), 99), v.end());
}

TEST(MapcatFlatten, DupAndNested) {
    auto dup = [](int x) { return std::vector<int>{x, x}; };
    EXPECT_EQ(mt::mapcat(std::vector{1, 2}, dup), (std::vector<int>{1, 1, 2, 2}));
    EXPECT_EQ(mt::flatten(std::vector<std::vector<int>>{{1, 2}, {}, {3}}),
              (std::vector<int>{1, 2, 3}));
}

TEST(Quantifiers, AllAnyNoneCount) {
    std::vector<int> v{2, 4, 6};
    EXPECT_TRUE(mt::all(v, [](int x) { return x % 2 == 0; }));
    EXPECT_TRUE(mt::any(v, [](int x) { return x == 4; }));
    EXPECT_TRUE(mt::none(v, [](int x) { return x < 0; }));
    EXPECT_EQ(mt::count_if(v, [](int x) { return x > 2; }), 2);
    EXPECT_TRUE(mt::all(std::vector<int>{}, [](int) { return false; }));
}

TEST(Orbit, CycleAndTerminating) {
    auto wrap = [](int x) { return (x + 1) % 5; };
    const int c = mt::collision_point(0, wrap);
    EXPECT_GE(c, 0);
    EXPECT_LT(c, 5);

    auto inc = [](int x) { return x + 1; };
    auto defined = [](int x) { return x < 10; };
    EXPECT_EQ(mt::collision_point(0, inc, defined), 10);
}

TEST(LowerBoundN, FirstNotLess) {
    std::vector<int> v{1, 2, 2, 4};
    EXPECT_EQ(mt::lower_bound_n(v.begin(), 4, 2), v.begin() + 1);
    EXPECT_EQ(mt::upper_bound_n(v.begin(), 4, 2), v.begin() + 3);
}

TEST(MergeSets, SortedAlgebra) {
    EXPECT_EQ(mt::merge(std::vector{1, 3}, std::vector{2, 4}), (std::vector<int>{1, 2, 3, 4}));
    EXPECT_EQ(mt::set_union(std::vector{1, 2}, std::vector{2, 3}), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(mt::set_intersection(std::vector{1, 2, 3}, std::vector{2, 3, 4}),
              (std::vector<int>{2, 3}));
}

TEST(ListOps, MapAccumGroupByNumeric) {
    auto running = [](int acc, int x) { return std::pair{acc + x, acc + x}; };
    auto [sum, prefixes] = mt::map_accum(std::vector{1, 2, 3}, 0, running);
    EXPECT_EQ(sum, 6);
    EXPECT_EQ(prefixes, (std::vector<int>{1, 3, 6}));

    auto groups = mt::group_by(std::vector{1, 1, 2, 2, 1}, std::equal_to<>{});
    ASSERT_EQ(groups.size(), 3u);
    EXPECT_EQ(groups[0], (std::vector<int>{1, 1}));

    EXPECT_EQ(mt::inner_product(std::vector{1, 2, 3}, std::vector{4, 5, 6}), 32);
    EXPECT_EQ(mt::adjacent_difference(std::vector{1, 3, 6, 10}), (std::vector<int>{1, 2, 3, 4}));

    auto [evens, odds] = mt::partition_copy(std::vector{1, 2, 3, 4}, [](int x) { return x % 2 == 0; });
    EXPECT_EQ(evens, (std::vector<int>{2, 4}));
    EXPECT_EQ(odds, (std::vector<int>{1, 3}));

    auto prod = mt::cartesian_product(std::vector{1, 2}, std::vector{'a', 'b'});
    EXPECT_EQ(prod.size(), 4u);
    EXPECT_EQ(prod.front(), (std::pair<int, char>{1, 'a'}));

    std::vector<int> perm{1, 2, 3};
    EXPECT_TRUE(mt::next_permutation(perm));
    EXPECT_EQ(perm, (std::vector<int>{1, 3, 2}));
}
