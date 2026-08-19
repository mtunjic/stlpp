#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <thread>
#include <vector>

namespace mt::par {

/// Sequential cutoff used by the adaptive parallel algorithms.
inline constexpr std::size_t default_cutoff = 4096;

[[nodiscard]] inline unsigned thread_count()
{
    const unsigned n = std::thread::hardware_concurrency();
    return n == 0 ? 1u : n;
}

/// Call `f(i)` for `i` in `[0, n)`. Uses up to `thread_count()` workers.
template <typename F>
void for_index(std::size_t n, F f)
{
    if (n == 0) {
        return;
    }
    const unsigned t = thread_count();
    if (n == 1 || t <= 1) {
        for (std::size_t i = 0; i < n; ++i) {
            f(i);
        }
        return;
    }
    const unsigned chunks = static_cast<unsigned>(std::min<std::size_t>(t, n));
    auto work = [n, chunks, f](unsigned c) {
        const std::size_t b = n * c / chunks;
        const std::size_t e = n * (c + 1) / chunks;
        for (std::size_t i = b; i < e; ++i) {
            f(i);
        }
    };
    std::vector<std::thread> workers;
    workers.reserve(chunks - 1);
    for (unsigned c = 0; c < chunks - 1; ++c) {
        workers.emplace_back(work, c);
    }
    work(chunks - 1);
    for (auto& w : workers) {
        w.join();
    }
}

/// Split `[first, last)` into up to `thread_count()` chunks and call `f(b, e)`
/// on each. Sequential when `n <= cutoff` or only one worker is available.
template <std::random_access_iterator I, typename F>
void for_chunks(I first, I last, F f, std::size_t cutoff = default_cutoff)
{
    using D = std::iter_difference_t<I>;
    const D n = last - first;
    if (n <= 0) {
        return;
    }
    const unsigned t = thread_count();
    if (static_cast<std::size_t>(n) <= cutoff || t <= 1) {
        f(first, last);
        return;
    }
    const unsigned chunks = std::min<unsigned>(t, static_cast<unsigned>(n));
    const D chunk = n / static_cast<D>(chunks);
    std::vector<std::thread> workers;
    workers.reserve(chunks - 1);
    I b = first;
    for (unsigned i = 0; i < chunks - 1; ++i) {
        I e = b + chunk;
        workers.emplace_back([f, b, e] { f(b, e); });
        b = e;
    }
    f(b, last);
    for (auto& w : workers) {
        w.join();
    }
}

}  // namespace mt::par
