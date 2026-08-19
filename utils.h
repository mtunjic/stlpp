#pragma once

#include "random.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <concepts>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <queue>
#include <ranges>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace mt {

/// Print a range as `[ a, b, c ]`.
///
/// Example:
///   mt::print(std::vector<int>{1, 2, 3});  // [ 1, 2, 3 ]
template <std::ranges::input_range R>
void print(const R& c, std::ostream& out = std::cout)
{
    if (std::ranges::empty(c)) {
        out << "(empty)\n";
        return;
    }
    auto it = std::ranges::begin(c);
    const auto last = std::ranges::end(c);
    out << "[ " << *it;
    ++it;
    for (; it != last; ++it) {
        out << ", " << *it;
    }
    out << " ]\n";
}

template <typename A, typename B>
void print(const std::pair<A, B>& p, std::ostream& out = std::cout)
{
    out << '(' << p.first << ", " << p.second << ")\n";
}

template <typename K, typename V, typename Comp, typename Alloc>
void print(const std::map<K, V, Comp, Alloc>& m, std::ostream& out = std::cout)
{
    out << '{';
    bool first = true;
    for (const auto& [k, v] : m) {
        if (!first) {
            out << ", ";
        }
        first = false;
        out << k << ": " << v;
    }
    out << "}\n";
}

template <typename T, typename Container, typename Comp>
void print(std::priority_queue<T, Container, Comp> pq, std::ostream& out = std::cout,
           std::size_t per_line = 8)
{
    std::size_t n = 0;
    while (!pq.empty()) {
        out << pq.top() << ' ';
        pq.pop();
        if (++n % per_line == 0) {
            out << '\n';
        }
    }
    if (n % per_line != 0) {
        out << '\n';
    }
}

/// High-resolution timer. `stop()` returns elapsed nanoseconds since `start()`.
///
/// Example:
///   mt::stopwatch t;
///   t.start();
///   work();
///   double ns = t.stop();
class stopwatch {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point t0_{clock::now()};

public:
    void start() { t0_ = clock::now(); }

    [[nodiscard]] double elapsed_ns() const
    {
        return static_cast<double>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - t0_).count());
    }

    double stop() { return elapsed_ns(); }
};

using timer = stopwatch;

inline void wait(std::istream& in = std::cin, std::ostream& out = std::cout)
{
    out << "Please enter to exit\n";
    in.clear();
    char ch{};
    in >> ch;
}

/// Shuffle a random-access range in place.
template <std::ranges::random_access_range R>
void randomize(R&& range)
{
    std::shuffle(std::ranges::begin(range), std::ranges::end(range), default_rng());
}

/// One uniform sample from `[lo, hi]` (inclusive for integers).
///
/// Example:
///   int n = mt::pick_a_number(5, 15);
template <Uniform_value T>
[[nodiscard]] T pick_a_number(T lo, T hi)
{
    T value{};
    T* first = &value;
    random(first, first + 1, lo, hi);
    return value;
}

/// Copy a range of pairs into `std::map` (later keys overwrite earlier ones).
///
/// Example:
///   auto m = mt::to_map(std::vector{std::pair{1, "a"}, std::pair{2, "b"}});
template <std::ranges::input_range R>
[[nodiscard]] auto to_map(R&& range)
{
    using E = std::remove_cvref_t<std::ranges::range_value_t<R>>;
    std::map<typename E::first_type, typename E::second_type> out;
    for (auto&& e : range) {
        out.insert_or_assign(e.first, e.second);
    }
    return out;
}

template <std::ranges::input_range R>
[[nodiscard]] auto to_list(R&& range)
{
    std::list<std::remove_cvref_t<std::ranges::range_value_t<R>>> out;
    for (auto&& x : range) {
        out.push_back(x);
    }
    return out;
}

template <std::size_t N, std::ranges::input_range R>
[[nodiscard]] auto to_array(R&& range)
{
    std::array<std::remove_cvref_t<std::ranges::range_value_t<R>>, N> out{};
    std::size_t i = 0;
    for (auto&& x : range) {
        if (i == N) {
            break;
        }
        out[i++] = x;
    }
    return out;
}

/// Sum of decimal digits. Negative integers use the absolute value.
///
/// Example:
///   mt::digit_sum(123);          // 6
///   mt::digit_sum("a1b2");       // 3
[[nodiscard]] inline int digit_sum(std::string_view s)
{
    int total = 0;
    for (char c : s) {
        if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
            total += c - '0';
        }
    }
    return total;
}

template <std::integral T>
[[nodiscard]] int digit_sum(T n)
{
    using U = std::make_unsigned_t<T>;
    U u = 0;
    if constexpr (std::is_signed_v<T>) {
        if (n < 0) {
            u = static_cast<U>(0) - static_cast<U>(n);
        } else {
            u = static_cast<U>(n);
        }
    } else {
        u = n;
    }
    int total = 0;
    do {
        total += static_cast<int>(u % 10);
        u /= 10;
    } while (u != 0);
    return total;
}

/// Time `f` over `iters` invocations. Returns elapsed nanoseconds.
///
/// Example:
///   auto ns = mt::bench([] { volatile int x = 1; (void)x; });
template <typename F>
double bench(F&& f, int iters = 1)
{
    stopwatch t;
    t.start();
    for (int i = 0; i < iters; ++i) {
        std::invoke(f);
    }
    return t.stop();
}

/// Join `t` in the destructor if it is still joinable (C++ Concurrency in Action).
class thread_guard {
    std::thread& t_;

public:
    explicit thread_guard(std::thread& t) : t_(t) {}
    ~thread_guard()
    {
        if (t_.joinable()) {
            t_.join();
        }
    }
    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
};

/// Join every joinable thread in `threads` on destruction.
class join_threads {
    std::vector<std::thread>& threads_;

public:
    explicit join_threads(std::vector<std::thread>& threads) : threads_(threads) {}
    ~join_threads()
    {
        for (std::thread& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
    join_threads(const join_threads&) = delete;
    join_threads& operator=(const join_threads&) = delete;
};

}  // namespace mt
