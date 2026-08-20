#include "demos.h"

#include "stlpp.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace stlpp_examples {
namespace {

void banner(std::string_view title)
{
    std::cout << "\n== " << title << " ==\n\n";
}

template <typename Clock = std::chrono::steady_clock>
[[nodiscard]] auto elapsed_us(typename Clock::time_point t0, typename Clock::time_point t1)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
}

enum class ExprTag : char { Num = 'N', Add = '+', Mul = '*', Sub = '-' };

struct Expr {
    ExprTag tag = ExprTag::Num;
    int value = 0;
    friend bool operator==(const Expr&, const Expr&) = default;
};

struct SortProbe {
    int value = 0;
    static long long moves;
    static long long comps;

    SortProbe() = default;
    explicit SortProbe(int v) : value(v) {}
    SortProbe(const SortProbe& o) : value(o.value) { ++moves; }
    SortProbe(SortProbe&& o) noexcept : value(o.value) { ++moves; }
    SortProbe& operator=(const SortProbe& o)
    {
        value = o.value;
        ++moves;
        return *this;
    }
    SortProbe& operator=(SortProbe&& o) noexcept
    {
        value = o.value;
        ++moves;
        return *this;
    }
};

long long SortProbe::moves = 0;
long long SortProbe::comps = 0;

struct DijkstraNode {
    int dist = 0;
    std::size_t vertex = 0;
    friend bool operator>(const DijkstraNode& a, const DijkstraNode& b)
    {
        if (a.dist != b.dist) {
            return a.dist > b.dist;
        }
        return a.vertex > b.vertex;
    }
};

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
        const auto powered = fib_exact(n);
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
    std::cout << "\n200× F_" << n << " ≡ " << sink << "  (" << elapsed_us(t0, t1)
              << " µs total)\n";
    std::cout << "Naive recurrence would need Θ(n) adds; power needs Θ(log n)\n"
                 "associative multiplies — same abstraction as modular\n"
                 "exponentiation and parallel prefix.\n";
}

void demo_pattern_hotspots()
{
    banner("Pattern hotspots — KMP matches + prefix scan + parallel sort");

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

void demo_expr_btree()
{
    banner("Bifurcate expression trees — traverse, evaluate, iso ≠ equal");

    auto make_num = [](int v) { return Expr{ExprTag::Num, v}; };
    auto make_op = [](ExprTag t) { return Expr{t, 0}; };

    // Tree A:  (2 * 3) + (10 - 4)  = 12
    mt::btree<Expr> a;
    const auto a_root = a.set_root(make_op(ExprTag::Add));
    const auto a_mul = a.add_left(a_root, make_op(ExprTag::Mul));
    const auto a_sub = a.add_right(a_root, make_op(ExprTag::Sub));
    a.add_left(a_mul, make_num(2));
    a.add_right(a_mul, make_num(3));
    a.add_left(a_sub, make_num(10));
    a.add_right(a_sub, make_num(4));

    // Tree B: same shape, different constants — isomorphic, not equal.
    mt::btree<Expr> b;
    const auto b_root = b.set_root(make_op(ExprTag::Add));
    const auto b_mul = b.add_left(b_root, make_op(ExprTag::Mul));
    const auto b_sub = b.add_right(b_root, make_op(ExprTag::Sub));
    b.add_left(b_mul, make_num(5));
    b.add_right(b_mul, make_num(7));
    b.add_left(b_sub, make_num(1));
    b.add_right(b_sub, make_num(1));

    // Tree C: 2 * (3 + (10 - 4)) — different topology (right-skewed).
    mt::btree<Expr> c;
    const auto c_root = c.set_root(make_op(ExprTag::Mul));
    c.add_left(c_root, make_num(2));
    const auto c_add = c.add_right(c_root, make_op(ExprTag::Add));
    c.add_left(c_add, make_num(3));
    const auto c_sub = c.add_right(c_add, make_op(ExprTag::Sub));
    c.add_left(c_sub, make_num(10));
    c.add_right(c_sub, make_num(4));

    auto eval = [](mt::btree_coord<Expr> root) {
        std::vector<int> stack;
        (void)mt::traverse_nonempty(root, [&](mt::visit v, mt::btree_coord<Expr> n) {
            if (v != mt::visit::post) {
                return;
            }
            const Expr& e = mt::source(n);
            if (e.tag == ExprTag::Num) {
                stack.push_back(e.value);
                return;
            }
            const int rhs = stack.back();
            stack.pop_back();
            const int lhs = stack.back();
            stack.pop_back();
            switch (e.tag) {
            case ExprTag::Add:
                stack.push_back(lhs + rhs);
                break;
            case ExprTag::Mul:
                stack.push_back(lhs * rhs);
                break;
            case ExprTag::Sub:
                stack.push_back(lhs - rhs);
                break;
            default:
                break;
            }
        });
        return stack.empty() ? 0 : stack.back();
    };

    auto infix = [](mt::btree_coord<Expr> root) {
        std::string out;
        (void)mt::traverse_nonempty(root, [&](mt::visit v, mt::btree_coord<Expr> n) {
            const Expr& e = mt::source(n);
            if (e.tag == ExprTag::Num) {
                if (v == mt::visit::pre) {
                    out += std::to_string(e.value);
                }
                return;
            }
            if (v == mt::visit::pre) {
                out += '(';
            } else if (v == mt::visit::in) {
                out += ' ';
                out += static_cast<char>(e.tag);
                out += ' ';
            } else {
                out += ')';
            }
        });
        return out;
    };

    std::cout << "A: " << infix(a.root()) << "  = " << eval(a.root()) << "\n";
    std::cout << "B: " << infix(b.root()) << "  = " << eval(b.root()) << "\n";
    std::cout << "C: " << infix(c.root()) << "  = " << eval(c.root()) << "\n\n";

    std::cout << "weight(A)=" << mt::weight(a.root())
              << "  height(A)=" << mt::height(a.root()) << "\n";
    std::cout << "bifurcate_isomorphic(A,B) = " << std::boolalpha
              << mt::bifurcate_isomorphic(a.root(), b.root()) << "\n";
    std::cout << "bifurcate_equal(A,B)      = "
              << mt::bifurcate_equal(a.root(), b.root()) << "\n";
    std::cout << "bifurcate_isomorphic(A,C) = "
              << mt::bifurcate_isomorphic(a.root(), c.root()) << "\n";
    std::cout << "bifurcate_equal(A,C)      = "
              << mt::bifurcate_equal(a.root(), c.root()) << "\n\n";
    std::cout << "Same shape ≠ same value (iso vs equal); post-order traverse\n"
                 "evaluates without allocating an AST visitor class.\n";
}

void demo_rope_zipper()
{
    banner("Rope + zipper — structural concat vs linear rebuild");

    constexpr int chunks = 2000;
    constexpr int chunk_len = 32;
    const std::string piece(chunk_len, 'x');

    const auto t0 = std::chrono::steady_clock::now();
    mt::rope r;
    for (int i = 0; i < chunks; ++i) {
        r = mt::concat(std::move(r), mt::rope{piece});
    }
    const auto t1 = std::chrono::steady_clock::now();
    const std::string flat = r.to_string();
    const auto t2 = std::chrono::steady_clock::now();

    std::string s;
    s.reserve(static_cast<std::size_t>(chunks) * chunk_len);
    const auto t3 = std::chrono::steady_clock::now();
    for (int i = 0; i < chunks; ++i) {
        s += piece;
    }
    const auto t4 = std::chrono::steady_clock::now();

    std::cout << "Built " << chunks << "×" << chunk_len << "-byte document\n";
    std::cout << "  rope concat loop:  " << elapsed_us(t0, t1) << " µs  (size="
              << r.size() << ")\n";
    std::cout << "  rope to_string:    " << elapsed_us(t1, t2) << " µs\n";
    std::cout << "  string += loop:    " << elapsed_us(t3, t4) << " µs  (size="
              << s.size() << ")\n";
    std::cout << "  strings match?     " << std::boolalpha << (flat == s) << "\n";
    std::cout << "  mid char rope.at:  '" << r.at(r.size() / 2) << "'\n\n";

    // Zipper: cursor edit without rewriting the whole buffer each keystroke.
    std::vector<char> seed(flat.begin(), flat.begin() + 40);
    mt::zipper<char> z{seed};
    for (int i = 0; i < 10; ++i) {
        z.move_right();
    }
    z.insert_right('[');
    z.move_right();
    z.insert_right(']');
    z.focus() = '*';
    const auto edited = z.to_vector();
    std::cout << "Zipper focus edit on a 40-char prefix:\n  ";
    for (char c : edited) {
        std::cout << c;
    }
    std::cout << "\n\nRope shares structure on concat; zipper localizes edits.\n"
                 "Together they model an editor buffer without Θ(n) copies per\n"
                 "append or cursor move.\n";
}

void demo_parallel_race()
{
    banner("Parallel map / reduce race — fold vs reduce vs map");

    constexpr std::size_t n = 4'000'000;
    std::vector<double> data(n);
    std::mt19937 rng{42};
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (double& x : data) {
        x = dist(rng);
    }

    const auto heavy = [](double x) {
        // Mildly expensive element work so map can show parallelism.
        return std::sin(x) * std::sin(x) + std::cos(x) * std::cos(x);
    };

    volatile double sink = 0;

    auto t0 = std::chrono::steady_clock::now();
    sink = mt::fold(data, 0.0, std::plus<>{});
    auto t1 = std::chrono::steady_clock::now();
    const auto fold_us = elapsed_us(t0, t1);
    const double fold_sum = sink;

    t0 = std::chrono::steady_clock::now();
    sink = mt::reduce(data, 0.0, std::plus<>{});
    t1 = std::chrono::steady_clock::now();
    const auto reduce_us = elapsed_us(t0, t1);
    const double reduce_sum = sink;

    std::vector<double> out_seq(n);
    t0 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        out_seq[i] = heavy(data[i]);
    }
    t1 = std::chrono::steady_clock::now();
    const auto seq_map_us = elapsed_us(t0, t1);

    std::vector<double> out_par(n);
    t0 = std::chrono::steady_clock::now();
    mt::map(data.begin(), data.end(), out_par.begin(), heavy);
    t1 = std::chrono::steady_clock::now();
    const auto par_map_us = elapsed_us(t0, t1);

    const double seq_checksum = mt::fold(out_seq, 0.0, std::plus<>{});
    const double par_checksum = mt::fold(out_par, 0.0, std::plus<>{});

    std::cout << "n = " << n << " doubles\n\n";
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "  fold  (left assoc):  " << std::setw(8) << fold_us
              << " µs   sum=" << fold_sum << "\n";
    std::cout << "  reduce (parallel):   " << std::setw(8) << reduce_us
              << " µs   sum=" << reduce_sum << "\n";
    std::cout << "  map sequential:      " << std::setw(8) << seq_map_us << " µs\n";
    std::cout << "  map parallel:        " << std::setw(8) << par_map_us << " µs\n";
    std::cout << "  checksums match?     " << std::boolalpha
              << (std::abs(seq_checksum - par_checksum) < 1e-6) << "\n\n";
    std::cout << "reduce requires an associative op (here +). fold always walks\n"
                 "left-to-right — pick the right tool for the algebra.\n";
}

void demo_cards_gather()
{
    banner("Deck choreography — gather + slide + partition_stable");

    // Suits: 0♥ 1♦ 2♣ 3♠. Ranks 1..13. Red = hearts/diamonds.
    struct Card {
        int suit;
        int rank;
    };
    auto face = [](const Card& c) {
        static constexpr char suits[] = {'H', 'D', 'C', 'S'};
        std::string s;
        s += suits[c.suit];
        s += std::to_string(c.rank);
        return s;
    };

    std::vector<Card> deck;
    deck.reserve(52);
    for (int s = 0; s < 4; ++s) {
        for (int r = 1; r <= 13; ++r) {
            deck.push_back({s, r});
        }
    }
    std::mt19937 rng{7};
    std::shuffle(deck.begin(), deck.end(), rng);

    auto print_row = [&](std::string_view label, const std::vector<Card>& d) {
        std::cout << label;
        for (std::size_t i = 0; i < d.size(); ++i) {
            if (i && i % 13 == 0) {
                std::cout << "\n" << std::string(label.size(), ' ');
            }
            std::cout << std::setw(4) << face(d[i]);
        }
        std::cout << "\n";
    };

    print_row("shuffled: ", deck);

    // Gather all reds around the middle of the deck (stable).
    const auto mid = deck.begin() + static_cast<std::ptrdiff_t>(deck.size() / 2);
    const auto [g0, g1] =
        mt::gather(deck.begin(), deck.end(), mid, [](const Card& c) { return c.suit < 2; });
    std::cout << "\nafter gather(red → mid): reds occupy ["
              << (g0 - deck.begin()) << ", " << (g1 - deck.begin()) << ")\n";
    print_row("gathered: ", deck);

    // Slide a 5-card hand from the front to just after the gather window.
    auto hand_lo = deck.begin();
    auto hand_hi = deck.begin() + 5;
    const auto [s0, s1] = mt::slide(hand_lo, hand_hi, g1);
    std::cout << "\nafter slide(first 5 → after reds): hand now ["
              << (s0 - deck.begin()) << ", " << (s1 - deck.begin()) << ")\n";
    print_row("slid:     ", deck);

    // Stable partition: blacks left, reds right (relative order preserved).
    (void)mt::partition_stable(deck.begin(), deck.end(),
                               [](const Card& c) { return c.suit >= 2; });
    std::cout << "\nafter partition_stable(black | red):\n";
    print_row("final:    ", deck);
    std::cout << "\ngather / slide / partition_stable rearrange in-place with\n"
                 "predictable stability — the same moves as a dealer’s hands.\n";
}

void demo_components()
{
    banner("Connected components — graph edges + disjoint_set");

    // 8×8 grid of sites; unite neighbors (and a few random long edges).
    constexpr int W = 8;
    constexpr int H = 8;
    constexpr int N = W * H;
    auto id = [](int x, int y) { return y * W + x; };

    mt::graph g(static_cast<std::size_t>(N));
    mt::disjoint_set dsu(static_cast<std::size_t>(N));

    auto link = [&](int a, int b) {
        g.add_undirected_edge(static_cast<std::size_t>(a), static_cast<std::size_t>(b));
        dsu.unite(static_cast<std::size_t>(a), static_cast<std::size_t>(b));
    };

    std::mt19937 rng{99};
    std::bernoulli_distribution keep(0.55);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (x + 1 < W && keep(rng)) {
                link(id(x, y), id(x + 1, y));
            }
            if (y + 1 < H && keep(rng)) {
                link(id(x, y), id(x, y + 1));
            }
        }
    }
    // A few “bridges” that may merge large regions.
    for (int i = 0; i < 6; ++i) {
        const int a = static_cast<int>(rng() % N);
        const int b = static_cast<int>(rng() % N);
        if (a != b) {
            link(a, b);
        }
    }

    std::vector<std::size_t> root_of(static_cast<std::size_t>(N));
    for (int i = 0; i < N; ++i) {
        root_of[static_cast<std::size_t>(i)] = dsu.find(static_cast<std::size_t>(i));
    }

    std::cout << "Grid " << W << "×" << H << " with random planar edges + bridges\n";
    std::cout << "components = " << dsu.components() << "\n";
    std::cout << "component sizes (sample of distinct roots):\n";
    std::vector<std::size_t> seen;
    for (std::size_t r : root_of) {
        if (std::find(seen.begin(), seen.end(), r) == seen.end()) {
            seen.push_back(r);
            std::cout << "  root " << std::setw(2) << r << "  size="
                      << dsu.component_size(r) << "\n";
        }
    }

    std::cout << "\nASCII map (letter = component id):\n";
    for (int y = 0; y < H; ++y) {
        std::cout << "  ";
        for (int x = 0; x < W; ++x) {
            const auto r = root_of[static_cast<std::size_t>(id(x, y))];
            const char label =
                static_cast<char>('A' + static_cast<int>(r % 26));
            std::cout << label << ' ';
        }
        std::cout << "\n";
    }

    const bool same_corner =
        dsu.connected(static_cast<std::size_t>(id(0, 0)),
                      static_cast<std::size_t>(id(W - 1, H - 1)));
    std::cout << "\ncorners connected? " << std::boolalpha << same_corner << "\n";
    std::cout << "degree(0) = " << g.neighbors(0).size() << "\n\n";
    std::cout << "Union-find answers connectivity in nearly α(n); the graph\n"
                 "keeps the actual adjacency for walks and rendering.\n";
}

void demo_lazy_pipeline()
{
    banner("Lazy iterator pipeline — numeric_range → filter → windows");

    // Integers [0, N) exist only as numeric_iterator until we filter.
    constexpr int N = 200;
    const auto nums = mt::numeric_range(0, N);

    std::vector<int> odds;
    for (int x : mt::filter(nums, [](int v) { return v % 2 != 0; })) {
        odds.push_back(x);
    }

    constexpr int k = 5;
    long long window_sum_total = 0;
    int window_count = 0;
    int max_window = 0;
    for (auto w : mt::windows(odds, k)) {
        const int s = mt::fold(w, 0, std::plus<>{});
        window_sum_total += s;
        max_window = std::max(max_window, s);
        ++window_count;
    }

    const double mean =
        window_count == 0 ? 0.0
                          : static_cast<double>(window_sum_total) / window_count;

    std::cout << "Source: numeric_range(0, " << N << ") — no heap vector for the\n"
                 "full domain; filter yields odds lazily, then we materialize\n"
                 "only the kept stream for random-access windows.\n\n";
    std::cout << "odds kept:           " << odds.size() << "\n";
    std::cout << "windows of " << k << ":       " << window_count << "\n";
    std::cout << "max window sum:      " << max_window << "\n";
    std::cout << "mean window sum:     " << std::fixed << std::setprecision(2)
              << mean << "\n";
    std::cout << "first window:        ";
    if (!odds.empty()) {
        auto first = *mt::windows(odds, k).begin();
        for (int x : first) {
            std::cout << x << ' ';
        }
    }
    std::cout << "\n\nComposition stays algorithm-shaped: range → filter →\n"
                 "windows → fold — not a nest of index arithmetic.\n";
}

void demo_adaptive_sort()
{
    banner("Adaptive sort_n — tiny vs large merge buffer");

    constexpr int n = 4096;
    std::vector<int> base(static_cast<std::size_t>(n));
    std::iota(base.begin(), base.end(), 0);
    std::mt19937 rng{123};
    std::shuffle(base.begin(), base.end(), rng);

    auto run_with_buf = [&](int n_buf) {
        std::vector<SortProbe> data;
        data.reserve(base.size());
        for (int v : base) {
            data.emplace_back(v);
        }
        std::vector<SortProbe> buffer(static_cast<std::size_t>(n_buf));
        SortProbe::moves = 0;
        SortProbe::comps = 0;
        const auto t0 = std::chrono::steady_clock::now();
        (void)mt::sort_n_adaptive(
            data.begin(), n, buffer.begin(), n_buf,
            [](const SortProbe& a, const SortProbe& b) {
                ++SortProbe::comps;
                return a.value < b.value;
            });
        const auto t1 = std::chrono::steady_clock::now();
        const bool sorted = std::is_sorted(
            data.begin(), data.end(),
            [](const SortProbe& a, const SortProbe& b) { return a.value < b.value; });
        return std::tuple{elapsed_us(t0, t1), SortProbe::moves, SortProbe::comps, sorted};
    };

    const auto [us_tiny, mv_tiny, cp_tiny, ok_tiny] = run_with_buf(2);
    const auto [us_big, mv_big, cp_big, ok_big] = run_with_buf(n / 2);

    std::cout << "n = " << n << " shuffled ints via sort_n_adaptive\n\n";
    std::cout << "  n_buf=2 (tiny):   " << std::setw(6) << us_tiny << " µs  moves="
              << mv_tiny << "  comps=" << cp_tiny << "  ok=" << std::boolalpha
              << ok_tiny << "\n";
    std::cout << "  n_buf=n/2 (fat):  " << std::setw(6) << us_big << " µs  moves="
              << mv_big << "  comps=" << cp_big << "  ok=" << ok_big << "\n\n";
    std::cout << "Same algorithm, different buffer budget: adaptive merge falls\n"
                 "back toward in-place work when scratch is scarce — EoP-style\n"
                 "complexity that tracks real machine resources.\n";
}

void demo_quadtree_query()
{
    banner("Quadtree — range query vs brute scan, nearest with refine");

    constexpr double extent = 256.0;
    constexpr int n_random = 20'000;
    constexpr int n_cluster = 400;
    mt::rect_2d world{0.0, 0.0, extent, extent};
    mt::rect_2d viewport{20.0, 20.0, 60.0, 60.0};
    const mt::point_2d probe{200.5, 180.25};

    std::mt19937 rng{2026};
    std::uniform_real_distribution<double> uni(0.0, extent * 0.999);
    std::normal_distribution<double> blob(40.0, 4.0);

    std::vector<mt::point_2d> cloud;
    cloud.reserve(static_cast<std::size_t>(n_random + n_cluster));
    for (int i = 0; i < n_random; ++i) {
        cloud.push_back({uni(rng), uni(rng)});
    }
    for (int i = 0; i < n_cluster; ++i) {
        const double x = std::clamp(blob(rng), 0.0, extent * 0.999);
        const double y = std::clamp(blob(rng), 0.0, extent * 0.999);
        cloud.push_back({x, y});
    }

    auto t0 = std::chrono::steady_clock::now();
    mt::quadtree tree(world);
    int inserted = 0;
    for (const auto& p : cloud) {
        inserted += static_cast<int>(tree.insert(p));
    }
    auto t1 = std::chrono::steady_clock::now();
    const auto insert_us = elapsed_us(t0, t1);

    t0 = std::chrono::steady_clock::now();
    const auto hits = tree.query(viewport);
    t1 = std::chrono::steady_clock::now();
    const auto query_us = elapsed_us(t0, t1);

    t0 = std::chrono::steady_clock::now();
    std::vector<mt::point_2d> brute;
    for (const auto& p : cloud) {
        if (viewport.contains(p)) {
            brute.push_back(p);
        }
    }
    t1 = std::chrono::steady_clock::now();
    const auto brute_us = elapsed_us(t0, t1);

    auto dist2 = [](mt::point_2d a, mt::point_2d b) {
        const double dx = a.x - b.x;
        const double dy = a.y - b.y;
        return dx * dx + dy * dy;
    };
    auto nearest_of = [&](const std::vector<mt::point_2d>& pts) -> mt::point_2d {
        mt::point_2d best = pts.front();
        double best_d = dist2(probe, best);
        for (const auto& p : pts) {
            const double d = dist2(probe, p);
            if (d < best_d) {
                best_d = d;
                best = p;
            }
        }
        return best;
    };

    // Expand an axis-aligned box until it contains at least one point. The first
    // hit can be a far corner (Chebyshev vs Euclidean), so refine at radius d.
    double half = 0.5;
    std::vector<mt::point_2d> box_hits;
    while (box_hits.empty() && half < extent * 2) {
        box_hits = tree.query({probe.x - half, probe.y - half, probe.x + half, probe.y + half});
        if (box_hits.empty()) {
            half *= 2.0;
        }
    }
    if (box_hits.empty()) {
        std::cout << "cloud empty — nothing to query.\n";
        return;
    }
    const mt::point_2d first_guess = nearest_of(box_hits);
    const double first_d = std::sqrt(dist2(probe, first_guess));
    const double refine_r = first_d + 1e-9;
    const auto refined_hits = tree.query(
        {probe.x - refine_r, probe.y - refine_r, probe.x + refine_r, probe.y + refine_r});
    const mt::point_2d refined = nearest_of(refined_hits);

    t0 = std::chrono::steady_clock::now();
    const mt::point_2d brute_nn = nearest_of(cloud);
    t1 = std::chrono::steady_clock::now();
    const auto nn_brute_us = elapsed_us(t0, t1);

    std::cout << "World [0," << extent << ")²  random=" << n_random
              << "  cluster@ (40,40) n=" << n_cluster << "\n";
    std::cout << "inserted " << inserted << " / " << cloud.size() << "  (" << insert_us
              << " µs)\n\n";
    std::cout << "Range query viewport [" << viewport.x0 << "," << viewport.y0 << ")–["
              << viewport.x1 << "," << viewport.y1 << ")\n";
    std::cout << "  quadtree.query:  " << hits.size() << " hits in " << query_us << " µs\n";
    std::cout << "  brute scan:      " << brute.size() << " hits in " << brute_us << " µs\n";
    std::cout << "  counts match?    " << std::boolalpha << (hits.size() == brute.size())
              << "\n\n";

    constexpr int gw = 24;
    constexpr int gh = 12;
    std::cout << "Occupancy of the viewport (" << gw << "×" << gh << " cells):\n";
    for (int row = gh - 1; row >= 0; --row) {
        std::cout << "  ";
        for (int col = 0; col < gw; ++col) {
            const double x0 =
                viewport.x0 + (viewport.x1 - viewport.x0) * static_cast<double>(col) / gw;
            const double x1 =
                viewport.x0 + (viewport.x1 - viewport.x0) * static_cast<double>(col + 1) / gw;
            const double y0 =
                viewport.y0 + (viewport.y1 - viewport.y0) * static_cast<double>(row) / gh;
            const double y1 =
                viewport.y0 + (viewport.y1 - viewport.y0) * static_cast<double>(row + 1) / gh;
            int cell = 0;
            for (const auto& p : hits) {
                if (p.x >= x0 && p.x < x1 && p.y >= y0 && p.y < y1) {
                    ++cell;
                }
            }
            std::cout << (cell == 0 ? '.' : (cell < 4 ? 'o' : '@'));
        }
        std::cout << "\n";
    }

    std::cout << "\nNearest to probe (" << probe.x << ", " << probe.y << "):\n";
    std::cout << "  first box (half=" << half << "):  (" << first_guess.x << ", "
              << first_guess.y << ")  d=" << first_d << "\n";
    std::cout << "  refined (Chebyshev r=d): (" << refined.x << ", " << refined.y
              << ")  d=" << std::sqrt(dist2(probe, refined)) << "\n";
    std::cout << "  brute Euclidean:         (" << brute_nn.x << ", " << brute_nn.y
              << ")  d=" << std::sqrt(dist2(probe, brute_nn)) << "  (" << nn_brute_us
              << " µs)\n";
    const bool nn_ok = dist2(refined, brute_nn) < 1e-18;
    std::cout << "  refined == brute?        " << std::boolalpha << nn_ok << "\n\n";
    std::cout << "A square of Chebyshev radius r can hit a far corner before a\n"
                 "closer axial neighbour — one extra query at r = d repairs that.\n"
                 "The tree still beats a full scan by pruning empty quadrants.\n";
}

void demo_dijkstra()
{
    banner("Dijkstra — decrease-key on mt::dynamic_pqueue");

    // Textbook decrease-key: HQ→Dock looks cheap until Gate relaxes Dock.
    //   HQ --10--> Dock --1--> Yard
    //   HQ --1-->  Gate --100--> Yard
    //   Gate --1--> Dock
    // Shortest HQ ↝ Yard is HQ → Gate → Dock → Yard = 3, not the 11-walk.
    constexpr std::size_t n = 4;
    const char* name[] = {"HQ", "Gate", "Dock", "Yard"};
    constexpr std::size_t HQ = 0;
    constexpr std::size_t Gate = 1;
    constexpr std::size_t Dock = 2;
    constexpr std::size_t Yard = 3;

    mt::graph g(n);
    std::vector<std::vector<int>> w(n, std::vector<int>(n, 0));
    auto link = [&](std::size_t u, std::size_t v, int weight) {
        g.add_edge(u, v);
        w[u][v] = weight;
    };
    link(HQ, Dock, 10);
    link(HQ, Gate, 1);
    link(Gate, Yard, 100);
    link(Gate, Dock, 1);
    link(Dock, Yard, 1);

    std::cout << "Directed weighted graph:\n";
    for (std::size_t u = 0; u < n; ++u) {
        std::cout << "  " << name[u] << ":";
        if (g.neighbors(u).empty()) {
            std::cout << "  (no outs)";
        }
        for (std::size_t v : g.neighbors(u)) {
            std::cout << "  -" << w[u][v] << "→ " << name[v];
        }
        std::cout << "\n";
    }

    constexpr int Inf = std::numeric_limits<int>::max() / 4;
    std::vector<int> dist(n, Inf);
    std::vector<std::size_t> parent(n, n);
    mt::dynamic_pqueue<DijkstraNode, std::greater<DijkstraNode>> pq;
    std::vector<mt::dynamic_pqueue<DijkstraNode, std::greater<DijkstraNode>>::handle> handle(n);

    dist[HQ] = 0;
    for (std::size_t v = 0; v < n; ++v) {
        handle[v] = pq.push({dist[v], v});
    }

    int pops = 0;
    int decrease_keys = 0;
    while (!pq.empty()) {
        const DijkstraNode u = pq.top();
        pq.pop();
        ++pops;
        if (u.dist != dist[u.vertex]) {
            continue;
        }
        for (std::size_t v : g.neighbors(u.vertex)) {
            const int cand = dist[u.vertex] + w[u.vertex][v];
            if (cand < dist[v]) {
                dist[v] = cand;
                parent[v] = u.vertex;
                pq.update(handle[v], {cand, v});
                ++decrease_keys;
            }
        }
    }

    std::cout << "\nShortest paths from HQ (decrease-key Dijkstra):\n";
    for (std::size_t v = 0; v < n; ++v) {
        std::cout << "  dist(" << name[v] << ") = ";
        if (dist[v] >= Inf) {
            std::cout << "∞\n";
            continue;
        }
        std::cout << dist[v] << "   path ";
        std::vector<std::size_t> walk;
        for (std::size_t x = v; x != n; x = parent[x]) {
            walk.push_back(x);
            if (x == HQ) {
                break;
            }
        }
        std::reverse(walk.begin(), walk.end());
        for (std::size_t i = 0; i < walk.size(); ++i) {
            std::cout << name[walk[i]] << (i + 1 < walk.size() ? " → " : "\n");
        }
    }

    std::cout << "\npops = " << pops << "  decrease-key updates = " << decrease_keys << "\n";
    std::cout << "naive first hop HQ→Dock (10) is later improved when Gate\n"
                 "relaxes Dock to 2 — that is the handle-stable update() the\n"
                 "heap exists for. std::priority_queue cannot decrease-key.\n";
}

void demo_text_pipeline()
{
    banner("Text pipeline — lines / starts_with / words / group_by");

    const std::string corpus =
        "# Notes from EoP\n"
        "\n"
        "ERROR: power(a, 0) must not loop\n"
        "Counted copy_n rejects n <= 0.\n"
        "# Algebra\n"
        "fold is always left-associated.\n"
        "ERROR: parallel reduce needs an associative op\n"
        "reduce_balanced is the tree-shaped cousin.\n"
        "fold is the sequential walk.\n";

    const auto rows = mt::lines(corpus);
    std::vector<std::string> headings;
    std::vector<std::string> errors;
    std::vector<std::string> body;
    for (const auto& row : rows) {
        if (row.empty()) {
            continue;
        }
        if (mt::starts_with(row, "# ")) {
            headings.push_back(row);
        } else if (mt::starts_with(row, "ERROR")) {
            errors.push_back(row);
        } else {
            body.push_back(row);
        }
    }

    std::cout << "Corpus: " << rows.size() << " lines (lines() keeps blanks for round-trip)\n";
    std::cout << "unlines(lines) == corpus? " << std::boolalpha
              << (mt::unlines(rows) == corpus) << "\n\n";

    std::cout << "Headings (starts_with \"# \"):\n";
    for (const auto& h : headings) {
        std::cout << "  " << h << "\n";
    }
    std::cout << "Errors (starts_with \"ERROR\"):\n";
    for (const auto& e : errors) {
        std::cout << "  " << e << "\n";
    }
    std::cout << "Body lines: " << body.size() << "\n";
    std::cout << "unlines(headings):\n---\n" << mt::unlines(headings) << "\n---\n\n";

    auto tokens = mt::words(mt::unlines(body));
    for (auto& t : tokens) {
        t = mt::to_lower_copy(t);
    }
    std::sort(tokens.begin(), tokens.end());
    const auto runs = mt::group_by(tokens, std::equal_to<>{});

    struct Freq {
        std::string word;
        int n = 0;
    };
    std::vector<Freq> freq;
    freq.reserve(runs.size());
    for (const auto& run : runs) {
        freq.push_back({run.front(), static_cast<int>(run.size())});
    }
    mt::psort(freq.begin(), freq.end(), [](const Freq& a, const Freq& b) {
        if (a.n != b.n) {
            return a.n > b.n;
        }
        return a.word < b.word;
    });

    std::cout << "Body words (lowercased) = " << tokens.size() << "  unique = " << runs.size()
              << "\n";
    std::cout << "Top frequencies (group_by after sort, then psort):\n";
    for (std::size_t i = 0; i < std::min<std::size_t>(8, freq.size()); ++i) {
        std::cout << "  " << std::setw(14) << freq[i].word << "  ×" << freq[i].n << "\n";
    }
    std::cout << "\nunwords(first 8 body tokens, original order) =\n  ";
    auto original = mt::words(mt::unlines(body));
    if (original.size() > 8) {
        original.resize(8);
    }
    std::cout << mt::unwords(original) << " …\n\n";
    std::cout << "Split / filter / group are the same generic algorithms as on\n"
                 "vectors of ints — the corpus is just another sequence.\n";
}

void demo_combinations()
{
    banner("Combinations — next_combination vs next_permutation");

    const std::vector<std::string> people{"Ada", "Bjorn", "Cora", "Drew", "Eve", "Fay"};
    constexpr int k = 3;
    std::vector<std::string> v = people;

    std::cout << "Choose " << k << " of " << people.size()
              << " (C(6,3)=20). Each step mutates the range in place:\n"
                 "  [selected | rest]  — both sides stay sorted (Hinnant).\n\n";

    int combo_count = 0;
    do {
        std::cout << "  { ";
        for (int i = 0; i < k; ++i) {
            std::cout << v[static_cast<std::size_t>(i)]
                      << (i + 1 < k ? ", " : " ");
        }
        std::cout << "}    rest:";
        for (std::size_t i = static_cast<std::size_t>(k); i < v.size(); ++i) {
            std::cout << " " << v[i];
        }
        std::cout << "\n";
        ++combo_count;
    } while (mt::next_combination(v.begin(), v.begin() + k, v.end()));

    std::cout << "\nwalked " << combo_count << " combinations; next_combination returned false\n";
    std::cout << "range restored to first combo / sorted rest? " << std::boolalpha
              << (v == people) << "\n\n";

    std::vector<std::string> trio{"Ada", "Bjorn", "Cora"};
    std::cout << "Same three people, now next_permutation (order matters, P(3,3)=6):\n";
    int perm_count = 0;
    do {
        std::cout << "  " << trio[0] << " → " << trio[1] << " → " << trio[2] << "\n";
        ++perm_count;
    } while (mt::next_permutation(trio));

    std::cout << "\nwalked " << perm_count << " permutations; wrapping restores sorted order? "
              << std::boolalpha << (trio == std::vector<std::string>{"Ada", "Bjorn", "Cora"})
              << "\n\n";
    std::cout << "Combinations ignore order (committee); permutations do not\n"
                 "(speaking order). Both are in-place generators — no extra\n"
                 "allocation per tuple, just a bidirectional rearrangement.\n";
}

}  // namespace stlpp_examples
