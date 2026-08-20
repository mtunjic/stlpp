#include "split.h"

#include <string>
#include <vector>

#include <benchmark/benchmark.h>

namespace {

std::string csv_line(int fields)
{
    std::string line;
    line.reserve(static_cast<std::size_t>(fields) * 8);
    for (int i = 0; i < fields; ++i) {
        if (i != 0) {
            line.push_back(',');
        }
        line += "field";
        line += std::to_string(i);
    }
    return line;
}

}  // namespace

static void BM_Split_CommaFields(benchmark::State& state)
{
    const auto line = csv_line(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        std::vector<std::string> words;
        mt::split(line, ",", words);
        benchmark::DoNotOptimize(words);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Split_CommaFields)->Arg(64)->Arg(1024)->Arg(1 << 14);

static void BM_Join_CommaFields(benchmark::State& state)
{
    const auto parts = mt::split_copy(csv_line(static_cast<int>(state.range(0))), ",");
    for (auto _ : state) {
        std::string joined = mt::join(parts, ",");
        benchmark::DoNotOptimize(joined);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Join_CommaFields)->Arg(64)->Arg(1024)->Arg(1 << 14);
