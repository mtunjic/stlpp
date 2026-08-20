#include "demos.h"

#include "stlpp.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
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

}  // namespace stlpp_examples
