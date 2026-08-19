#!/usr/bin/env python3
"""Run STL++ Google Benchmarks, keep dated JSON, plot one comparison graph.

Each capture writes:
  bench/history/stlpp-bench-YYYY-MM-DD.json          raw gbench JSON
  bench/stlpp-bench-YYYY-MM-DD.svg                   this run (cpu ns)
  bench/stlpp-bench-history.svg                      all runs, for comparison
  bench/history.json                                 compact index of every run

Re-running on the same calendar day appends HHMMSS so history is never overwritten.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BENCH_DIR = ROOT / "bench"
HISTORY_DIR = BENCH_DIR / "history"

# One graph: same size so measurements are comparable. Higher-level algos
# (scan/fold/reduce/map) plus the binary_counter vs std::accumulate pair.
GRAPH_FILTER = (
    r"BM_(Scan_Plus|Fold_Sum|Reduce_Sum|Map_Double|"
    r"BinaryCounter_PlusReduce|StdAccumulate_Plus)/1024$"
)

UNIT_TO_NS = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}


def to_ns(value: float, unit: str) -> float:
    return float(value) * UNIT_TO_NS.get(unit, 1.0)


def find_binary() -> Path:
    candidates = [
        ROOT / "build" / "stlpp_bench.exe",
        ROOT / "build" / "stlpp_bench",
        ROOT / "build" / "Release" / "stlpp_bench.exe",
        ROOT / "build" / "RelWithDebInfo" / "stlpp_bench.exe",
        ROOT / "build" / "Debug" / "stlpp_bench.exe",
        ROOT / "build" / "algorithms_bench.exe",
        ROOT / "build" / "algorithms_bench",
    ]
    for path in candidates:
        if path.is_file():
            return path
    raise FileNotFoundError(
        "stlpp_bench not found. Build it first:\n"
        "  cmake --build build --target stlpp_bench"
    )


def stamp_paths() -> tuple[str, Path, Path]:
    now = datetime.now()
    day = now.strftime("%Y-%m-%d")
    json_path = HISTORY_DIR / f"stlpp-bench-{day}.json"
    if json_path.exists():
        day = now.strftime("%Y-%m-%d_%H%M%S")
        json_path = HISTORY_DIR / f"stlpp-bench-{day}.json"
    svg_path = BENCH_DIR / f"stlpp-bench-{day}.svg"
    return day, json_path, svg_path


def run_gbench(binary: Path, out_json: Path, min_time: str, extra: list[str]) -> dict:
    out_json.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(binary),
        f"--benchmark_filter={GRAPH_FILTER}",
        f"--benchmark_min_time={min_time}",
        "--benchmark_format=json",
        f"--benchmark_out={out_json}",
        "--benchmark_out_format=json",
        *extra,
    ]
    print(" ".join(cmd), file=sys.stderr)
    subprocess.run(cmd, check=True, cwd=ROOT)
    return json.loads(out_json.read_text(encoding="utf-8"))


def compact_run(raw: dict, run_id: str, source: str) -> dict:
    ctx = raw.get("context") or {}
    benches = []
    for b in raw.get("benchmarks") or []:
        name = b.get("name") or ""
        if name.endswith("_mean") or name.endswith("_median") or name.endswith("_stddev"):
            continue
        if b.get("run_type") == "aggregate":
            continue
        unit = b.get("time_unit") or "ns"
        benches.append(
            {
                "name": name,
                "cpu_time_ns": round(to_ns(b["cpu_time"], unit), 3),
                "real_time_ns": round(to_ns(b["real_time"], unit), 3),
                "iterations": b.get("iterations"),
                "items_per_second": b.get("items_per_second"),
            }
        )
    benches.sort(key=lambda row: row["name"])
    return {
        "id": run_id,
        "source": source,
        "date": ctx.get("date"),
        "host_name": ctx.get("host_name"),
        "num_cpus": ctx.get("num_cpus"),
        "mhz_per_cpu": ctx.get("mhz_per_cpu"),
        "library_build_type": ctx.get("library_build_type"),
        "benchmarks": benches,
    }


def load_index() -> dict:
    index_path = BENCH_DIR / "history.json"
    if index_path.is_file():
        return json.loads(index_path.read_text(encoding="utf-8"))
    return {"title": "STL++ Google Benchmark history", "filter": GRAPH_FILTER, "runs": []}


def upsert_run(index: dict, run: dict) -> dict:
    runs = [r for r in index.get("runs", []) if r.get("id") != run["id"]]
    runs.append(run)
    runs.sort(key=lambda r: r["id"])
    index["filter"] = GRAPH_FILTER
    index["runs"] = runs
    return index


def short_name(name: str) -> str:
    return re.sub(r"^BM_", "", name).replace("/1024", "")


def svg_escape(text: str) -> str:
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def write_bar_svg(path: Path, title: str, caption: str, labels: list[str], values: list[float]) -> None:
    width, height = 920, 420
    left, right, top, bottom = 72, 24, 48, 88
    plot_w = width - left - right
    plot_h = height - top - bottom
    peak = max(values) if values else 1.0
    peak = peak * 1.12 if peak > 0 else 1.0
    n = max(len(values), 1)
    gap = 12
    bar_w = max(18, (plot_w - gap * (n + 1)) / n)
    ticks = 4

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#f7f7f5"/>',
        f'<text x="{left}" y="28" font-family="ui-sans-serif, system-ui, sans-serif" font-size="16" fill="#1a1a1a">{svg_escape(title)}</text>',
        f'<text x="{left}" y="44" font-family="ui-sans-serif, system-ui, sans-serif" font-size="11" fill="#666">{svg_escape(caption)}</text>',
        f'<text x="16" y="{top + plot_h / 2}" font-family="ui-sans-serif, system-ui, sans-serif" font-size="11" fill="#666" transform="rotate(-90 16,{top + plot_h / 2})">CPU time (ns)</text>',
    ]
    for i in range(ticks + 1):
        y = top + plot_h - (plot_h * i / ticks)
        val = peak * i / ticks
        parts.append(
            f'<line x1="{left}" y1="{y:.1f}" x2="{width - right}" y2="{y:.1f}" stroke="#ddd" stroke-width="1"/>'
        )
        parts.append(
            f'<text x="{left - 8}" y="{y + 4:.1f}" text-anchor="end" font-family="ui-sans-serif, system-ui, sans-serif" font-size="10" fill="#666">{val:.0f}</text>'
        )
    for i, (label, value) in enumerate(zip(labels, values)):
        x = left + gap + i * (bar_w + gap)
        h = (value / peak) * plot_h if peak else 0
        y = top + plot_h - h
        parts.append(
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_w:.1f}" height="{h:.1f}" fill="#2563eb"/>'
        )
        parts.append(
            f'<text x="{x + bar_w / 2:.1f}" y="{y - 6:.1f}" text-anchor="middle" font-family="ui-sans-serif, system-ui, sans-serif" font-size="10" fill="#1a1a1a">{value:.0f}</text>'
        )
        parts.append(
            f'<text x="{x + bar_w / 2:.1f}" y="{height - 36}" text-anchor="middle" font-family="ui-sans-serif, system-ui, sans-serif" font-size="10" fill="#333" transform="rotate(-28 {x + bar_w / 2:.1f},{height - 36})">{svg_escape(label)}</text>'
        )
    parts.append(
        f'<text x="{left}" y="{height - 10}" font-family="ui-sans-serif, system-ui, sans-serif" font-size="10" fill="#888">Source: stlpp_bench · {svg_escape(GRAPH_FILTER)}</text>'
    )
    parts.append("</svg>")
    path.write_text("\n".join(parts), encoding="utf-8")


def write_history_svg(path: Path, runs: list[dict]) -> None:
    names: list[str] = []
    seen = set()
    for run in runs:
        for b in run["benchmarks"]:
            if b["name"] not in seen:
                seen.add(b["name"])
                names.append(b["name"])
    dates = [r["id"] for r in runs]
    width, height = 960, 440
    left, right, top, bottom = 72, 24, 56, 64
    plot_w = width - left - right
    plot_h = height - top - bottom
    n_dates = max(len(dates), 1)
    n_series = max(len(names), 1)
    group_w = plot_w / n_dates
    bar_w = max(6, (group_w * 0.7) / n_series)
    palette = ["#2563eb", "#0f766e", "#b45309", "#7c3aed", "#be123c", "#365314"]
    peak = 1.0
    for run in runs:
        for b in run["benchmarks"]:
            peak = max(peak, b["cpu_time_ns"])
    peak *= 1.12

    lookup = {r["id"]: {b["name"]: b["cpu_time_ns"] for b in r["benchmarks"]} for r in runs}

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#f7f7f5"/>',
        f'<text x="{left}" y="28" font-family="ui-sans-serif, system-ui, sans-serif" font-size="16" fill="#1a1a1a">STL++ CPU time by run (history)</text>',
        '<text x="72" y="44" font-family="ui-sans-serif, system-ui, sans-serif" font-size="11" fill="#666">Google Benchmark cpu_time, n=1024 · grouped by capture date</text>',
        f'<text x="16" y="{top + plot_h / 2}" font-family="ui-sans-serif, system-ui, sans-serif" font-size="11" fill="#666" transform="rotate(-90 16,{top + plot_h / 2})">CPU time (ns)</text>',
    ]
    for i in range(5):
        y = top + plot_h - (plot_h * i / 4)
        val = peak * i / 4
        parts.append(
            f'<line x1="{left}" y1="{y:.1f}" x2="{width - right}" y2="{y:.1f}" stroke="#ddd" stroke-width="1"/>'
        )
        parts.append(
            f'<text x="{left - 8}" y="{y + 4:.1f}" text-anchor="end" font-family="ui-sans-serif, system-ui, sans-serif" font-size="10" fill="#666">{val:.0f}</text>'
        )
    for di, date in enumerate(dates):
        gx = left + di * group_w + group_w * 0.15
        for si, name in enumerate(names):
            value = lookup[date].get(name, 0.0)
            h = (value / peak) * plot_h if peak else 0
            x = gx + si * bar_w
            y = top + plot_h - h
            color = palette[si % len(palette)]
            parts.append(
                f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_w - 2:.1f}" height="{h:.1f}" fill="{color}"/>'
            )
        parts.append(
            f'<text x="{left + di * group_w + group_w / 2:.1f}" y="{height - 36}" text-anchor="middle" font-family="ui-sans-serif, system-ui, sans-serif" font-size="11" fill="#333">{svg_escape(date)}</text>'
        )
    legend_x = left
    for si, name in enumerate(names):
        color = palette[si % len(palette)]
        parts.append(f'<rect x="{legend_x}" y="{height - 22}" width="10" height="10" fill="{color}"/>')
        parts.append(
            f'<text x="{legend_x + 14}" y="{height - 13}" font-family="ui-sans-serif, system-ui, sans-serif" font-size="10" fill="#333">{svg_escape(short_name(name))}</text>'
        )
        legend_x += 8 * len(short_name(name)) + 28
    parts.append("</svg>")
    path.write_text("\n".join(parts), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--min-time", default="0.1s", help="Google Benchmark --benchmark_min_time")
    parser.add_argument("--binary", type=Path, help="Path to stlpp_bench")
    parser.add_argument("gbench_args", nargs="*", help="Extra args forwarded to stlpp_bench")
    args = parser.parse_args()

    BENCH_DIR.mkdir(parents=True, exist_ok=True)
    HISTORY_DIR.mkdir(parents=True, exist_ok=True)

    binary = args.binary or find_binary()
    run_id, json_path, svg_path = stamp_paths()
    raw = run_gbench(binary, json_path, args.min_time, args.gbench_args)
    run = compact_run(raw, run_id, str(json_path.relative_to(ROOT)).replace("\\", "/"))

    index = upsert_run(load_index(), run)
    (BENCH_DIR / "history.json").write_text(json.dumps(index, indent=2) + "\n", encoding="utf-8")

    labels = [short_name(b["name"]) for b in run["benchmarks"]]
    values = [b["cpu_time_ns"] for b in run["benchmarks"]]
    write_bar_svg(
        svg_path,
        f"STL++ CPU time · {run_id}",
        f"Google Benchmark cpu_time · n=1024 · {run.get('library_build_type') or 'build'} · {run.get('num_cpus')} CPUs",
        labels,
        values,
    )
    write_history_svg(BENCH_DIR / "stlpp-bench-history.svg", index["runs"])

    print(f"wrote {json_path.relative_to(ROOT)}")
    print(f"wrote {svg_path.relative_to(ROOT)}")
    print(f"wrote bench/stlpp-bench-history.svg  ({len(index['runs'])} run(s))")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        print(f"benchmark binary failed: {exc}", file=sys.stderr)
        raise SystemExit(exc.returncode)
    except FileNotFoundError as exc:
        print(exc, file=sys.stderr)
        raise SystemExit(1)
