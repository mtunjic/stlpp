# Benchmark history

One comparable graph: Google Benchmark **cpu_time (ns)** at `n=1024` for scan, fold, reduce, map, binary_counter, and `std::accumulate`.

Each capture keeps a dated file. Later runs are appended, not overwritten (same calendar day gets `_HHMMSS`).

```sh
cmake --build build --target stlpp_bench
python scripts/bench_history.py --min-time 0.1s
```

| File | Role |
|------|------|
| `bench/history/stlpp-bench-YYYY-MM-DD.json` | Raw gbench JSON for that run |
| `bench/stlpp-bench-YYYY-MM-DD.svg` | Bar graph of that run |
| `bench/stlpp-bench-history.svg` | All runs on one graph (comparison) |
| `bench/history.json` | Compact index used for comparison |

![STL++ CPU time 2026-08-19](stlpp-bench-2026-08-19.svg)

Current capture: **2026-08-19**, host MAKINA, **debug** binary, 20 CPUs. Recapture in Release before treating numbers as ship numbers.
