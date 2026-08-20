// Iterator view examples not covered by generic law tests (zip_view, manual unfold loops, …).

#include "stlpp.h"

#include <optional>
#include <vector>

#include <gtest/gtest.h>

TEST(TransformIterator, Squares)
{
    std::vector<int> v{1, 2, 3};
    std::vector<int> out;
    for (int x : mt::transform(v, [](int n) { return n * n; })) {
        out.push_back(x);
    }
    EXPECT_EQ(out, (std::vector<int>{1, 4, 9}));
}

TEST(FilterIterator, Evens)
{
    std::vector<int> v{1, 2, 3, 4, 5};
    std::vector<int> out;
    for (int x : mt::filter(v, [](int n) { return n % 2 == 0; })) {
        out.push_back(x);
    }
    EXPECT_EQ(out, (std::vector<int>{2, 4}));
}

TEST(ZipView, StopsAtShorter)
{
    std::vector<int> a{1, 2};
    std::vector<int> b{10, 20, 30};
    std::vector<std::pair<int, int>> out;
    for (auto p : mt::zip_view(a, b)) {
        out.push_back(p);
    }
    EXPECT_EQ(out, (std::vector<std::pair<int, int>>{{1, 10}, {2, 20}}));
}

TEST(WindowChunks, SlidingAndBlocking)
{
    std::vector<int> v{1, 2, 3, 4, 5};
    std::vector<std::vector<int>> wins;
    for (auto w : mt::windows(v, 2)) {
        wins.emplace_back(w.begin(), w.end());
    }
    EXPECT_EQ(wins, (std::vector<std::vector<int>>{{1, 2}, {2, 3}, {3, 4}, {4, 5}}));

    std::vector<std::vector<int>> ch;
    for (auto c : mt::chunks(v, 2)) {
        ch.emplace_back(c.begin(), c.end());
    }
    EXPECT_EQ(ch, (std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5}}));
}

TEST(CycleIterator, RepeatsFiniteTimes)
{
    std::vector<int> v{1, 2};
    std::vector<int> out;
    for (int x : mt::cycle(v, 3)) {
        out.push_back(x);
    }
    EXPECT_EQ(out, (std::vector<int>{1, 2, 1, 2, 1, 2}));
    EXPECT_TRUE(mt::cycle(v, 0).begin() == mt::cycle(v, 0).end());
}

TEST(UnfoldIterator, ManualCountdownLoop)
{
    auto step = [](int n) -> std::optional<std::pair<int, int>> {
        if (n <= 0) {
            return std::nullopt;
        }
        return std::pair{n, n - 1};
    };
    std::vector<int> out;
    for (mt::unfold_iterator it{step, 3}; it != mt::unfold_iterator<decltype(step), int>{}; ++it) {
        out.push_back(*it);
    }
    EXPECT_EQ(out, (std::vector<int>{3, 2, 1}));
}
