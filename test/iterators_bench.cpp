#include "stlpp.h"

#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_NumericIterator_Sum(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        int sum = std::accumulate(mt::numeric_iterator<int>{0}, mt::numeric_iterator<int>{n}, 0);
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_NumericIterator_Sum)->Arg(1024)->Arg(1 << 16);

static void BM_StepIterator_Stride(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        auto s = mt::step(values, 2);
        int sum = 0;
        for (int x : s) {
            sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * (state.range(0) / 2));
}
BENCHMARK(BM_StepIterator_Stride)->Arg(1024)->Arg(1 << 16);

static void BM_EdgeIterator_Walk(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        int sum = 0;
        for (auto [a, b] : mt::edges(values)) {
            sum += a + b;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_EdgeIterator_Walk)->Arg(1024)->Arg(1 << 16);
