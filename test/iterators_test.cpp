#include "stlpp.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

static_assert(std::random_access_iterator<mt::numeric_iterator<int>>);
static_assert(std::random_access_iterator<mt::step_iterator<int*>>);
static_assert(std::forward_iterator<mt::edge_iterator<int*>>);

TEST(NumericIterator, CopiesHalfOpenRange) {
    std::vector<int> v;
    std::copy(mt::numeric_iterator<int>{0}, mt::numeric_iterator<int>{4}, std::back_inserter(v));
    EXPECT_EQ(v, (std::vector<int>{0, 1, 2, 3}));
}

TEST(NumericIterator, RangeForAndDistance) {
    std::vector<int> v;
    for (int x : mt::numeric_range(10, 13)) {
        v.push_back(x);
    }
    EXPECT_EQ(v, (std::vector<int>{10, 11, 12}));
    EXPECT_EQ(mt::numeric_range(0, 10).size(), 10);
    EXPECT_EQ(mt::numeric_iterator<int>{7} - mt::numeric_iterator<int>{2}, 5);
}

TEST(StepIterator, StridesByTwo) {
    std::vector<int> v{0, 1, 2, 3, 4, 5};
    std::vector<int> out;
    auto s = mt::step(v, 2);
    std::copy(s.begin(), s.end(), std::back_inserter(out));
    EXPECT_EQ(out, (std::vector<int>{0, 2, 4}));
}

TEST(StepIterator, CountWhenLengthNotMultipleOfStep) {
    std::vector<int> v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> out;
    auto s = mt::step(v.begin(), v.end(), 3);
    std::copy(s.begin(), s.end(), std::back_inserter(out));
    EXPECT_EQ(out, (std::vector<int>{0, 3, 6, 9}));
    EXPECT_EQ(s.end() - s.begin(), 4);
}

TEST(EdgeIterator, ConsecutivePairs) {
    std::vector<int> v{1, 2, 3, 4};
    std::vector<std::pair<int, int>> out;
    for (auto [from, to] : mt::edges(v)) {
        out.emplace_back(from, to);
    }
    EXPECT_EQ(out, (std::vector<std::pair<int, int>>{{1, 2}, {2, 3}, {3, 4}}));
}

TEST(EdgeIterator, EmptyAndSingletonHaveNoEdges) {
    std::vector<int> empty;
    EXPECT_TRUE(mt::edges(empty).begin() == mt::edges(empty).end());

    std::vector<int> one{7};
    auto e = mt::edges(one);
    EXPECT_TRUE(e.begin() == e.end());
}

TEST(IteratorAdapter, CountingBasisIsRandomAccess) {
    struct counting_basis {
        using state_type = int;
        using value_type = int;
        using reference = int;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        int n = 0;
        explicit counting_basis(int x = 0) : n(x) {}
        [[nodiscard]] int state() const { return n; }
        [[nodiscard]] int deref() const { return n; }
        void increment() { ++n; }
        void decrement() { --n; }
        void offset(std::ptrdiff_t d) { n += static_cast<int>(d); }
        [[nodiscard]] std::ptrdiff_t difference(const counting_basis& other) const
        {
            return static_cast<std::ptrdiff_t>(n - other.n);
        }
    };

    using I = mt::iterator<counting_basis>;
    std::vector<int> v;
    std::copy(I{0}, I{3}, std::back_inserter(v));
    EXPECT_EQ(v, (std::vector<int>{0, 1, 2}));
    EXPECT_EQ(I{5} - I{2}, 3);
}
