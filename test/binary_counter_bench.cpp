#include "binary_counter.h"

#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>

namespace {

std::vector<int> iota_n(int n)
{
    std::vector<int> values(static_cast<std::size_t>(n));
    std::iota(values.begin(), values.end(), 1);
    return values;
}

}  // namespace

static void BM_BinaryCounter_PlusReduce(benchmark::State& state)
{
    const auto values = iota_n(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        mt::binary_counter<int, std::plus<int>> counter(std::plus<int>{}, 0);
        for (int value : values) {
            counter.add(value);
        }
        int result = counter.reduce();
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_BinaryCounter_PlusReduce)->Arg(64)->Arg(1024)->Arg(1 << 16);

static void BM_StdAccumulate_Plus(benchmark::State& state)
{
    const auto values = iota_n(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        int sum = std::accumulate(values.begin(), values.end(), 0);
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdAccumulate_Plus)->Arg(64)->Arg(1024)->Arg(1 << 16);
