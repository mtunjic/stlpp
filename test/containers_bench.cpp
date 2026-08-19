#include "stlpp.h"

#include <functional>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

static void BM_Tape_Walk(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        mt::tape<int> t{0};
        int sum = 0;
        for (std::size_t i = 0; i < n; ++i) {
            t.write(static_cast<int>(i));
            sum += t.read();
            t.right();
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Tape_Walk)->Arg(1024)->Arg(1 << 16);

static void BM_Htable_InsertFind(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        mt::htable<int, int> t;
        for (int i = 0; i < n; ++i) {
            t.insert(i, i);
        }
        int sum = 0;
        for (int i = 0; i < n; ++i) {
            sum += *t.find(i);
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Htable_InsertFind)->Arg(1024)->Arg(1 << 14);

static void BM_Vector2d_Fill(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        mt::vector_2d<int> m(n, n, 0);
        for (std::size_t r = 0; r < n; ++r) {
            for (std::size_t c = 0; c < n; ++c) {
                m(r, c) = static_cast<int>(r + c);
            }
        }
        benchmark::DoNotOptimize(m(n - 1, n - 1));
    }
    state.SetItemsProcessed(state.iterations() * state.range(0) * state.range(0));
}
BENCHMARK(BM_Vector2d_Fill)->Arg(32)->Arg(128);

static void BM_Rope_ConcatFlatten(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    const std::string chunk(16, 'a');
    for (auto _ : state) {
        mt::rope r;
        for (int i = 0; i < n; ++i) {
            r.append(chunk);
        }
        auto s = r.to_string();
        benchmark::DoNotOptimize(s.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Rope_ConcatFlatten)->Arg(64)->Arg(1024);

static void BM_DynamicPQueue_PushPop(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        mt::dynamic_pqueue<int, std::greater<int>> q;
        for (int i = 0; i < n; ++i) {
            q.push(n - i);
        }
        int sum = 0;
        while (!q.empty()) {
            sum += q.top();
            q.pop();
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_DynamicPQueue_PushPop)->Arg(1024)->Arg(1 << 14);

static void BM_ArraySlice_Sum(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    std::vector<int> values(n);
    for (std::size_t i = 0; i < n; ++i) {
        values[i] = static_cast<int>(i);
    }
    for (auto _ : state) {
        mt::array_slice<int> s{values};
        int sum = 0;
        for (int x : s) {
            sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ArraySlice_Sum)->Arg(1024)->Arg(1 << 16);

static void BM_Grid_Neighbors(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    mt::grid<int> g(n, n, 0);
    for (auto _ : state) {
        std::size_t count = 0;
        for (std::size_t r = 0; r < n; ++r) {
            for (std::size_t c = 0; c < n; ++c) {
                count += g.neighbors4(r, c).size();
            }
        }
        benchmark::DoNotOptimize(count);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0) * state.range(0));
}
BENCHMARK(BM_Grid_Neighbors)->Arg(32)->Arg(64);

static void BM_Graph_Build(benchmark::State& state) {
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        mt::graph g(n);
        for (std::size_t i = 1; i < n; ++i) {
            g.add_edge(i - 1, i);
        }
        benchmark::DoNotOptimize(g.neighbors(0).size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Graph_Build)->Arg(1024)->Arg(1 << 14);

static void BM_TsQueue_PushPop(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        mt::ts_queue<int> q;
        for (int i = 0; i < n; ++i) {
            q.push(i);
        }
        int sum = 0;
        while (auto x = q.try_pop()) {
            sum += *x;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_TsQueue_PushPop)->Arg(1024)->Arg(1 << 14);

static void BM_Quadtree_InsertQuery(benchmark::State& state) {
    const auto n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        mt::quadtree q({0, 0, 1024, 1024});
        for (int i = 0; i < n; ++i) {
            q.insert({static_cast<double>(i % 1024), static_cast<double>((i * 7) % 1024)});
        }
        auto hits = q.query({0, 0, 128, 128});
        benchmark::DoNotOptimize(hits.size());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_Quadtree_InsertQuery)->Arg(256)->Arg(4096);
