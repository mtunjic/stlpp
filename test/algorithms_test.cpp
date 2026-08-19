#include "algorithms.h"
#include <gtest/gtest.h>
#include <benchmark/benchmark.h>

// Test for odd function
TEST(AlgorithmTests, OddFunction) {
    EXPECT_TRUE(odd(3));
    EXPECT_FALSE(odd(4));
}

// Test for half function
TEST(AlgorithmTests, HalfFunction) {
    EXPECT_EQ(half(6), 3);
    EXPECT_EQ(half(7), 3);
}

// Test for high_byte function
TEST(AlgorithmTests, HighByteFunction) {
    uint16_t value = 0x1234;
    EXPECT_EQ(high_byte(value), 0x12);
}

// Test for advance function
TEST(AlgorithmTests, AdvanceFunction) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto it = vec.begin();
    mt::advance(it, 2);
    EXPECT_EQ(*it, 3);
}

// Test for find_if_n function
TEST(AlgorithmTests, FindIfNFunction) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto it = mt::find_if_n(vec.begin(), vec.end(), 2, [](int x){ return x % 2 == 0; });
    EXPECT_EQ(*it, 4);
}

// Test for find_backward_if function
TEST(AlgorithmTests, FindBackwardIfFunction) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto it = mt::find_backward_if(vec.begin(), vec.end(), 2, [](int x){ return x % 2 == 0; });
    EXPECT_EQ(*it, 2);
}

// Benchmark for odd function
BENCHMARK(OddBenchmark) {
    for (int i = 0; i < 10000; ++i) {
        odd(i);
    }
};

// Benchmark for half function
BENCHMARK(HalfBenchmark) {
    for (int i = 0; i < 10000; ++i) {
        half(i);
    }
};

// Benchmark for advance function
BENCHMARK(AdvanceBenchmark) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto it = vec.begin();
    mt::advance(it, 2);
}

// Benchmark for find_if_n function
BENCHMARK(FindIfNBenchmark) {
    std::vector<int> vec(10000);
    std::iota(vec.begin(), vec.end(), 0);
    auto it = mt::find_if_n(vec.begin(), vec.end(), 5000, [](int x){ return x % 2 == 0; });
}

// Benchmark for find_backward_if function
BENCHMARK(FindBackwardIfBenchmark) {
    std::vector<int> vec(10000);
    std::iota(vec.begin(), vec.end(), 0);
    auto it = mt::find_backward_if(vec.begin(), vec.end(), 5000, [](int x){ return x % 2 == 0; });
}

// Benchmark for high_byte function
BENCHMARK(HighByteBenchmark) {
    uint16_t value = 0x1234;
    for (int i = 0; i < 10000; ++i) {
        high_byte(value);
    }
}