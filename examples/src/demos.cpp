#include "demos.h"

#include "stlpp.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace stlpp_examples {
namespace {

void banner(std::string_view title)
{
    std::cout << "\n== " << title << " ==\n\n";
}

}  // namespace

void demo_orbit_rho()
{
    banner("Orbit forensics — Floyd + connection_point (EoP ch. 2)");

    // Hand-built ρ-shaped map on {0..15}: stem 0→1→2→3, cycle 3→7→10→12→3.
    // Same geometry as hash-chain / PRNG orbits — recovered without storing edges.
    constexpr int start = 0;
    auto f = [](int x) {
        static constexpr int next[] = {
            /*0*/ 1, /*1*/ 2, /*2*/ 3, /*3*/ 7, /*4*/ 4, /*5*/ 5, /*6*/ 6,
            /*7*/ 10, /*8*/ 8, /*9*/ 9, /*10*/ 12, /*11*/ 11, /*12*/ 3,
            /*13*/ 13, /*14*/ 14, /*15*/ 15,
        };
        return next[x];
    };

    std::cout << "Map:  explicit next[] on {0..15} (ρ: 0→1→2→3⇄7→10→12)\n";
    std::cout << "Start: x₀ = " << start << "\n\n";

    const int collide = mt::collision_point(start, f);
    const int connect = mt::connection_point(start, f);
    const auto stem = mt::transformation_distance(start, connect, f);
    const auto cycle = mt::transformation_distance(f(connect), connect, f) + 1;

    std::cout << "collision_point  → " << collide
              << "  (some point on the cycle after tortoise/hare meet)\n";
    std::cout << "connection_point → " << connect << "  (entry to the cycle)\n";
    std::cout << "stem length μ    → " << stem << "\n";
    std::cout << "cycle length λ   → " << cycle << "\n";
    std::cout << "circular(start)? → " << std::boolalpha << mt::circular(start, f) << "\n\n";

    std::cout << "Stem walk:\n  ";
    int x = start;
    for (std::size_t i = 0; i < stem; ++i) {
        std::cout << x << " → ";
        x = f(x);
    }
    std::cout << "[" << connect << "]";
    std::cout << "\nCycle walk (λ steps):\n  ";
    x = connect;
    for (std::size_t i = 0; i < cycle; ++i) {
        std::cout << x;
        x = f(x);
        if (i + 1 < cycle) {
            std::cout << " → ";
        }
    }
    std::cout << " → (" << connect << "…)\n\n";
    std::cout << "Why it matters: the same algorithms detect loops in iterator\n"
                 "graphs, hash chains, and state machines — without building the\n"
                 "whole orbit in memory.\n";
}

void demo_matrix_fibonacci()
{
    banner("Algebraic power — Fibonacci via 2×2 matrix (mt::power)");

    // [[1,1],[1,0]]^n ≡ [[F_{n+1}, F_n], [F_n, F_{n-1}]] (mod M).
    // Associative matrix multiply + Stepanov power ⇒ F_n in O(log n) products.
    constexpr long long mod = 1'000'000'007LL;
    struct Mat2 {
        long long a{}, b{}, c{}, d{};
    };
    const auto mul = [](Mat2 x, Mat2 y) {
        auto mm = [](long long u, long long v) { return (u * v) % mod; };
        auto aa = [](long long u, long long v) { return (u + v) % mod; };
        return Mat2{aa(mm(x.a, y.a), mm(x.b, y.c)), aa(mm(x.a, y.b), mm(x.b, y.d)),
                    aa(mm(x.c, y.a), mm(x.d, y.c)), aa(mm(x.c, y.b), mm(x.d, y.d))};
    };
    const Mat2 fib_step{1, 1, 1, 0};
    const Mat2 identity{1, 0, 0, 1};

    auto fib_mod = [&](unsigned n) -> long long {
        if (n == 0) {
            return 0;
        }
        return mt::power(fib_step, n, mul, identity).b;
    };

    // Exact small values (no modular reduction needed for display).
    auto fib_exact = [](unsigned n) -> long long {
        long long a = 0;
        long long b = 1;
        for (unsigned i = 0; i < n; ++i) {
            const long long c = a + b;
            a = b;
            b = c;
        }
        return a;
    };

    std::cout << "Exact F_n (sanity vs naive loop):\n";
    for (unsigned n : {0u, 1u, 2u, 5u, 10u, 20u, 30u}) {
        const auto powered = [&] {
            if (n == 0) {
                return 0LL;
            }
            // Exact power without mod for small n: use long double? Prefer compare
            // modular power against naive for n<=30 where F_30 fits in 32-bit.
            return fib_exact(n);
        }();
        std::cout << "  F_" << std::setw(2) << n << " = " << powered;
        if (n > 0) {
            std::cout << "   (power mod M also " << fib_mod(n) << ")";
        }
        std::cout << "\n";
    }

    std::cout << "\nLarge n with mt::power (mod " << mod << "):\n";
    for (unsigned n : {40u, 60u, 90u, 1'000'000u}) {
        std::cout << "  F_" << n << " ≡ " << fib_mod(n) << "\n";
    }

    constexpr unsigned n = 1'000'000u;
    const auto t0 = std::chrono::steady_clock::now();
    volatile long long sink = 0;
    for (int i = 0; i < 200; ++i) {
        sink = fib_mod(n);
    }
    const auto t1 = std::chrono::steady_clock::now();
    const auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "\n200× F_" << n << " ≡ " << sink << "  (" << us << " µs total)\n";
    std::cout << "Naive recurrence would need Θ(n) adds; power needs Θ(log n)\n"
                 "associative multiplies — same abstraction as modular\n"
                 "exponentiation and parallel prefix.\n";
}

void demo_pattern_hotspots()
{
    banner("Pattern hotspots — KMP matches + prefix scan + parallel sort");

    // Overlapping motif search (KMP), then a density scan and a parallel sort
    // of window scores — a small “bioinformatics / log forensics” pipeline.
    const std::string genome =
        "ACGTACACACGTACGTACACACGTXXXXACACACGTACGTACAC"
        "ACGTACACACGTACACACGTACGTACACACGTACGTACACACGT"
        "NNNNACACACGTACGTACACACGTACACACGTACGTACAC";
    const std::string motif = "ACAC";

    const auto hits = mt::matches(genome, motif);
    std::cout << "Text length n=" << genome.size() << ", motif=\"" << motif
              << "\" (m=" << motif.size() << ")\n";
    std::cout << "Overlapping KMP hits: " << hits.size() << "\n  offsets: ";
    for (std::size_t i = 0; i < hits.size(); ++i) {
        std::cout << hits[i] << (i + 1 < hits.size() ? ", " : "\n");
    }

    constexpr int window = 16;
    std::vector<int> density(genome.size(), 0);
    for (std::size_t h : hits) {
        density[h] += 1;
    }
    const auto prefix = mt::scan(density, std::plus<>{});

    struct WindowScore {
        int start;
        int count;
    };
    std::vector<WindowScore> windows;
    for (int i = 0; i + window <= static_cast<int>(genome.size()); ++i) {
        const int right = prefix[static_cast<std::size_t>(i + window - 1)];
        const int left = i == 0 ? 0 : prefix[static_cast<std::size_t>(i - 1)];
        windows.push_back({i, right - left});
    }

    mt::psort(windows.begin(), windows.end(),
              [](const WindowScore& a, const WindowScore& b) {
                  if (a.count != b.count) {
                      return a.count > b.count;
                  }
                  return a.start < b.start;
              });

    std::cout << "\nTop " << window << "-base windows by motif density "
                 "(psort of scan scores):\n";
    for (std::size_t i = 0; i < std::min<std::size_t>(5, windows.size()); ++i) {
        const auto& w = windows[i];
        std::cout << "  #" << (i + 1) << "  start=" << std::setw(3) << w.start
                  << "  hits_in_window=" << w.count << "  snippet=\""
                  << genome.substr(static_cast<std::size_t>(w.start),
                                   static_cast<std::size_t>(window))
                  << "\"\n";
    }

    const int total_hits =
        mt::reduce_balanced(density.begin(), density.end(), std::plus<>{}, 0);
    std::cout << "\nreduce_balanced(density) = " << total_hits
              << " (matches hits.size())\n";
    std::cout << "Pipeline: matches → indicator → scan → window scores → psort.\n"
                 "Each step is a reusable generic algorithm, not a one-off loop.\n";
}

}  // namespace stlpp_examples
