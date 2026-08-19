#include "stlpp.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_Apply_Square(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        mt::apply(values, [](int x) { return x + 1; });
        benchmark::DoNotOptimize(values.front());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Apply_Square)->Arg(1024)->Arg(1 << 16);

static void BM_ReverseN_Vector(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        mt::reverse_n(values.begin(), static_cast<int>(values.size()));
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ReverseN_Vector)->Arg(1024)->Arg(1 << 16);

static void BM_SteinGcd(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        int g = mt::stein_gcd(n, n / 2 + 1);
        benchmark::DoNotOptimize(g);
    }
}
BENCHMARK(BM_SteinGcd)->Arg(1024)->Arg(1 << 20);

static void BM_MinMax(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        auto [lo, hi] = mt::min_max(values.begin(), values.end());
        benchmark::DoNotOptimize(*lo);
        benchmark::DoNotOptimize(*hi);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MinMax)->Arg(1024)->Arg(1 << 16);

static void BM_Zip(benchmark::State& state) {
    std::vector<int> a(static_cast<std::size_t>(state.range(0)));
    std::vector<int> b(static_cast<std::size_t>(state.range(0)));
    std::iota(a.begin(), a.end(), 0);
    std::iota(b.begin(), b.end(), 1);
    for (auto _ : state) {
        auto z = mt::zip(a, b);
        benchmark::DoNotOptimize(z.front());
        benchmark::DoNotOptimize(z.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Zip)->Arg(1024)->Arg(1 << 14);

static void BM_MedianSort(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> proto(n);
    std::iota(proto.begin(), proto.end(), 0);
    std::reverse(proto.begin(), proto.end());
    for (auto _ : state) {
        auto values = proto;
        mt::median_sort(values);
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MedianSort)->Arg(256)->Arg(4096);

static void BM_Matches_Kmp(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    const std::string text(n, 'a');
    const std::string pat = "aaa";
    for (auto _ : state) {
        auto hits = mt::matches(text, pat);
        benchmark::DoNotOptimize(hits.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Matches_Kmp)->Arg(1024)->Arg(1 << 16);

static void BM_ShiftBy(benchmark::State& state) {
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        mt::shift_by(values, 1);
        benchmark::DoNotOptimize(values.front());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ShiftBy)->Arg(1024)->Arg(1 << 16);
