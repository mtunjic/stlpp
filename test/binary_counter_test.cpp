#include "binary_counter.h"

#include <array>
#include <functional>
#include <numeric>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

struct Min {
    int operator()(int a, int b) const { return a < b ? a : b; }
};

}  // namespace

TEST(AddToCounter, EmptyRange_ReturnsCarry) {
    std::vector<int> slots;
    const int carry =
        mt::add_to_counter(slots.begin(), slots.end(), std::plus<int>{}, 7, 0);
    EXPECT_EQ(carry, 7);
    EXPECT_TRUE(slots.empty());
}

TEST(AddToCounter, EmptySlot_StoresCarryAndAbsorbs) {
    std::array<int, 2> slots{0, 0};

    const int leftover =
        mt::add_to_counter(slots.begin(), slots.end(), std::plus<int>{}, 5, 0);

    EXPECT_EQ(leftover, 0);
    EXPECT_EQ(slots[0], 5);
    EXPECT_EQ(slots[1], 0);
}

TEST(AddToCounter, OccupiedSlot_CombinesAndCarries) {
    std::array<int, 2> slots{3, 0};

    const int leftover =
        mt::add_to_counter(slots.begin(), slots.end(), std::plus<int>{}, 4, 0);

    EXPECT_EQ(leftover, 0);
    EXPECT_EQ(slots[0], 0);
    EXPECT_EQ(slots[1], 7);
}

TEST(AddToCounter, FullCounter_ReturnsOverflowCarry) {
    std::array<int, 1> slots{1};

    const int leftover =
        mt::add_to_counter(slots.begin(), slots.end(), std::plus<int>{}, 2, 0);

    EXPECT_EQ(leftover, 3);
    EXPECT_EQ(slots[0], 0);
}

TEST(ReduceCounter, AllZero_ReturnsZero) {
    const std::array<int, 3> slots{0, 0, 0};
    EXPECT_EQ(mt::reduce_counter(slots.begin(), slots.end(), std::plus<int>{}, 0), 0);
}

TEST(ReduceCounter, SkipsLeadingZeros_FoldsOccupiedSlots) {
    const std::array<int, 4> slots{0, 2, 0, 5};
    EXPECT_EQ(mt::reduce_counter(slots.begin(), slots.end(), std::plus<int>{}, 0), 7);
}

TEST(BinaryCounter, AddThenReduce_EqualsLinearFold) {
    mt::binary_counter<int, std::plus<int>> counter(std::plus<int>{}, 0);
    const std::vector<int> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (int value : values) {
        counter.add(value);
    }

    const int expected = std::accumulate(values.begin(), values.end(), 0);
    EXPECT_EQ(counter.reduce(), expected);
    EXPECT_FALSE(counter.empty());
}

TEST(BinaryCounter, Empty_ReduceReturnsZero) {
    mt::binary_counter<int, std::plus<int>> counter(std::plus<int>{}, 0);
    EXPECT_TRUE(counter.empty());
    EXPECT_EQ(counter.reduce(), 0);
}

TEST(BinaryCounter, MinOp_ReduceReturnsMinimum) {
    constexpr int kEmpty = 1'000'000;
    mt::binary_counter<int, Min> counter(Min{}, kEmpty);

    counter.add(9);
    counter.add(3);
    counter.add(7);
    counter.add(3);

    EXPECT_EQ(counter.reduce(), 3);
}

TEST(BinaryCounter, StringPlus_ConcatenatesAssociatively) {
    mt::binary_counter<std::string, std::plus<std::string>> counter(
        std::plus<std::string>{}, std::string{});

    counter.add("a");
    counter.add("b");
    counter.add("c");
    counter.add("d");

    EXPECT_EQ(counter.reduce(), "abcd");
}
