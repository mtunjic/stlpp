#include "stlpp.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_Map_Double(benchmark::State& state)
{
    std::vector<int> proto(static_cast<std::size_t>(state.range(0)));
    std::iota(proto.begin(), proto.end(), 0);
    for (auto _ : state) {
        auto out = mt::map(proto, [](int x) { return x * 2; });
        benchmark::DoNotOptimize(out.front());
        benchmark::DoNotOptimize(out.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Map_Double)->Arg(1024)->Arg(1 << 16);

static void BM_Reduce_Sum(benchmark::State& state)
{
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        auto s = mt::reduce(values, 0, std::plus<>{});
        benchmark::DoNotOptimize(s);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Reduce_Sum)->Arg(1024)->Arg(1 << 16);

static void BM_Fold_Sum(benchmark::State& state)
{
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    for (auto _ : state) {
        auto s = mt::fold(values, 0, std::plus<>{});
        benchmark::DoNotOptimize(s);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Fold_Sum)->Arg(1024)->Arg(1 << 16);

static void BM_Pfind_Last(benchmark::State& state)
{
    std::vector<int> values(static_cast<std::size_t>(state.range(0)));
    std::iota(values.begin(), values.end(), 0);
    const int target = static_cast<int>(state.range(0) - 1);
    for (auto _ : state) {
        auto it = mt::pfind(values, target);
        benchmark::DoNotOptimize(*it);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Pfind_Last)->Arg(1024)->Arg(1 << 16);

static void BM_Psort_Reverse(benchmark::State& state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> proto(n);
    std::iota(proto.begin(), proto.end(), 0);
    std::reverse(proto.begin(), proto.end());
    for (auto _ : state) {
        auto values = proto;
        mt::psort(values);
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Psort_Reverse)->Arg(1024)->Arg(1 << 16);

static void BM_PartialBucketsort_Reverse(benchmark::State& state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> proto(n);
    std::iota(proto.begin(), proto.end(), 0);
    std::reverse(proto.begin(), proto.end());
    for (auto _ : state) {
        auto values = proto;
        mt::partial_bucketsort(values);
        benchmark::DoNotOptimize(values.front());
        benchmark::DoNotOptimize(values.back());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_PartialBucketsort_Reverse)->Arg(1024)->Arg(1 << 16);
