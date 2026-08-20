#pragma once

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iterator>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace mt::par {

/// Sequential cutoff used by the adaptive parallel algorithms.
inline constexpr std::size_t default_cutoff = 4096;

[[nodiscard]] inline unsigned thread_count()
{
    const unsigned n = std::thread::hardware_concurrency();
    return n == 0 ? 1u : n;
}

[[nodiscard]] inline unsigned worker_chunks(std::size_t n)
{
    if (n == 0) {
        return 0;
    }
    // Use size_t min so a large `n` is not truncated to 0 via `unsigned(n)`.
    return static_cast<unsigned>(std::min<std::size_t>(thread_count(), n));
}

[[nodiscard]] inline std::size_t split_at(std::size_t n, unsigned c, unsigned chunks)
{
    if (chunks == 0) {
        return 0;
    }
    const std::size_t q = n / chunks;
    const std::size_t r = n % chunks;
    return q * static_cast<std::size_t>(c) + r * static_cast<std::size_t>(c) / chunks;
}

template <std::random_access_iterator I>
[[nodiscard]] std::pair<I, I> chunk_bounds(I first, std::size_t n, unsigned c, unsigned chunks)
{
    using D = std::iter_difference_t<I>;
    return {first + static_cast<D>(split_at(n, c, chunks)),
            first + static_cast<D>(split_at(n, c + 1, chunks))};
}

template <typename F>
void launch_chunks(unsigned chunks, F work)
{
    if (chunks == 0) {
        return;
    }
    std::vector<std::thread> workers;
    workers.reserve(chunks - 1);
    auto join = [&] {
        for (auto& w : workers) {
            if (w.joinable()) {
                w.join();
            }
        }
    };
    try {
        for (unsigned c = 0; c < chunks - 1; ++c) {
            workers.emplace_back(work, c);
        }
        work(chunks - 1);
    } catch (...) {
        join();
        throw;
    }
    join();
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
    const unsigned chunks = worker_chunks(n);
    std::mutex mu;
    std::exception_ptr eptr;
    launch_chunks(chunks, [n, chunks, f, &mu, &eptr](unsigned c) {
        try {
            const std::size_t b = split_at(n, c, chunks);
            const std::size_t e = split_at(n, c + 1, chunks);
            for (std::size_t i = b; i < e; ++i) {
                f(i);
            }
        } catch (...) {
            std::lock_guard<std::mutex> lock(mu);
            if (!eptr) {
                eptr = std::current_exception();
            }
        }
    });
    if (eptr) {
        std::rethrow_exception(eptr);
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
    const unsigned chunks = worker_chunks(static_cast<std::size_t>(n));
    if (chunks == 0) {
        return;
    }
    std::mutex mu;
    std::exception_ptr eptr;
    launch_chunks(chunks, [first, n, chunks, f, &mu, &eptr](unsigned c) {
        try {
            auto [b, e] = chunk_bounds(first, static_cast<std::size_t>(n), c, chunks);
            f(b, e);
        } catch (...) {
            std::lock_guard<std::mutex> lock(mu);
            if (!eptr) {
                eptr = std::current_exception();
            }
        }
    });
    if (eptr) {
        std::rethrow_exception(eptr);
    }
}

}  // namespace mt::par
