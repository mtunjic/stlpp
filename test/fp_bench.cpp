#include "stlpp.h"

#include <functional>
#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_Power_Mul(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        auto v = mt::power(2, n, std::multiplies<int>{});
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Power_Mul)->Arg(10)->Arg(20);

static void BM_Scan_Plus(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 1);
    for (auto _ : state) {
        auto out = mt::scan(values, std::plus<>{});
        benchmark::DoNotOptimize(out.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Scan_Plus)->Arg(1024)->Arg(1 << 16);

static void BM_FilterIterator_Evens(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        int sum = 0;
        for (int x : mt::filter(values, [](int n) { return n % 2 == 0; })) {
            sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_FilterIterator_Evens)->Arg(1024)->Arg(1 << 16);

static void BM_Merge_Halves(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> a(n / 2);
    std::vector<int> b(n - n / 2);
    std::iota(a.begin(), a.end(), 0);
    std::iota(b.begin(), b.end(), static_cast<int>(n / 2));
    for (auto _ : state) {
        auto out = mt::merge(a, b);
        benchmark::DoNotOptimize(out.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Merge_Halves)->Arg(1024)->Arg(1 << 16);

static void BM_DisjointSet_UniteFind(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        mt::disjoint_set ds(n);
        for (std::size_t i = 1; i < n; ++i) {
            ds.unite(i - 1, i);
        }
        benchmark::DoNotOptimize(ds.connected(0, n - 1));
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_DisjointSet_UniteFind)->Arg(1024)->Arg(1 << 14);
