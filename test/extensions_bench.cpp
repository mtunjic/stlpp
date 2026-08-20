#include "stlpp.h"

#include <string>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_KeepIf(benchmark::State& state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> proto(n);
    for (std::size_t i = 0; i < n; ++i) {
        proto[i] = static_cast<int>(i % 10);
    }
    for (auto _ : state) {
        auto values = proto;
        mt::keep_if(values, [](int x) { return x < 5; });
        benchmark::DoNotOptimize(values.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_KeepIf)->Arg(1024)->Arg(1 << 16);

static void BM_Sanitize(benchmark::State& state)
{
    const std::string src(static_cast<std::size_t>(state.range(0)), 'A');
    for (auto _ : state) {
        auto s = src;
        mt::sanitize(s);
        benchmark::DoNotOptimize(s.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Sanitize)->Arg(1024)->Arg(1 << 16);

static void BM_ToLower(benchmark::State& state)
{
    const std::string src(static_cast<std::size_t>(state.range(0)), 'Z');
    for (auto _ : state) {
        auto s = src;
        mt::to_lower(s);
        benchmark::DoNotOptimize(s.front());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ToLower)->Arg(1024)->Arg(1 << 16);

static void BM_Distinct(benchmark::State& state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> proto(n);
    for (std::size_t i = 0; i < n; ++i) {
        proto[i] = static_cast<int>(i % 64);
    }
    for (auto _ : state) {
        auto values = proto;
        mt::distinct(values);
        benchmark::DoNotOptimize(values.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Distinct)->Arg(1024)->Arg(1 << 14);

static void BM_Gsub(benchmark::State& state)
{
    const std::string src(static_cast<std::size_t>(state.range(0)), 'a');
    for (auto _ : state) {
        auto s = src;
        mt::gsub(s, "aa", "b");
        benchmark::DoNotOptimize(s.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Gsub)->Arg(1024)->Arg(1 << 16);

static void BM_DigitSum(benchmark::State& state)
{
    const auto n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        int s = mt::digit_sum(n);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_DigitSum)->Arg(123456789)->Arg(1 << 20);
