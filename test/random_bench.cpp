#include "random.h"

#include <numeric>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_Random_UniformInt(benchmark::State& state) {
    std::mt19937_64 rng{1};
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        mt::random(values, 0, 1'000'000, rng);
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Random_UniformInt)->Arg(64)->Arg(1024)->Arg(1 << 16);

static void BM_RandomIota_Shuffle(benchmark::State& state) {
    std::mt19937_64 rng{1};
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        mt::random_iota(values, rng);
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_RandomIota_Shuffle)->Arg(64)->Arg(1024)->Arg(1 << 16);

static void BM_StdIota(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        std::iota(values.begin(), values.end(), 0);
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdIota)->Arg(64)->Arg(1024)->Arg(1 << 16);
