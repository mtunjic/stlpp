#pragma once

#include "concepts.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <utility>

namespace mt {

// random / random_iota
//
// Flow (random): resolve [lo, hi] via minmax, then write a uniform sample at
// each position. Integers use inclusive uniform_int_distribution; floating
// values use uniform_real_distribution. Equal bounds fill with that constant.
//
// Flow (random_iota): write 0, 1, ..., n-1 then Fisher–Yates shuffle.
//
// Iterator strength: forward (random); random-access (random_iota, shuffle).
// Complexity: O(n) draws / swaps. The old implementation used a Gaussian with
// (min, max) as (mean, stddev) and std::random_shuffle — both are gone.
//
// Example:
//   std::vector<double> rd(8);
//   mt::random(rd, 0.5, 5.0);     // uniform samples in [0.5, 5)
//   std::array<int, 52> deck{};
//   mt::random_iota(deck);        // shuffled 0..51

[[nodiscard]] inline std::mt19937_64& default_rng()
{
    thread_local std::mt19937_64 rng{std::random_device{}()};
    return rng;
}

template <typename T>
concept Uniform_value = Number<T> && !std::same_as<T, bool>;

template <std::forward_iterator I, std::sentinel_for<I> S, Uniform_value T, typename URBG>
    requires std::uniform_random_bit_generator<std::remove_cvref_t<URBG>> &&
             std::indirectly_writable<I, T>
void random(I first, S last, T lo, T hi, URBG&& rng)
{
    const auto [a, b] = std::minmax(lo, hi);
    if (a == b) {
        for (; first != last; ++first) {
            *first = a;
        }
        return;
    }

    if constexpr (std::integral<T>) {
        using Dist_int = std::conditional_t<sizeof(T) == 1, int, T>;
        std::uniform_int_distribution<Dist_int> dist(static_cast<Dist_int>(a),
                                                     static_cast<Dist_int>(b));
        for (; first != last; ++first) {
            *first = static_cast<T>(dist(rng));
        }
    } else {
        std::uniform_real_distribution<T> dist(a, b);
        for (; first != last; ++first) {
            *first = dist(rng);
        }
    }
}

template <std::forward_iterator I, std::sentinel_for<I> S, Uniform_value T>
    requires std::indirectly_writable<I, T>
void random(I first, S last, T lo, T hi)
{
    random(first, last, lo, hi, default_rng());
}

template <std::ranges::forward_range R, Uniform_value T, typename URBG>
    requires std::uniform_random_bit_generator<std::remove_cvref_t<URBG>> &&
             std::indirectly_writable<std::ranges::iterator_t<R>, T>
void random(R&& range, T lo, T hi, URBG&& rng)
{
    random(std::ranges::begin(range), std::ranges::end(range), lo, hi, std::forward<URBG>(rng));
}

template <std::ranges::forward_range R, Uniform_value T>
    requires std::indirectly_writable<std::ranges::iterator_t<R>, T>
void random(R&& range, T lo, T hi)
{
    random(std::forward<R>(range), lo, hi, default_rng());
}

template <std::ranges::forward_range R, Uniform_value T>
    requires std::indirectly_writable<std::ranges::iterator_t<R>, T>
void random(R&& range, std::pair<T, T> bounds)
{
    random(std::forward<R>(range), bounds.first, bounds.second);
}

template <std::random_access_iterator I, std::sentinel_for<I> S, typename URBG>
    requires std::uniform_random_bit_generator<std::remove_cvref_t<URBG>> &&
             std::incrementable<Value_type<I>>
void random_iota(I first, S last, URBG&& rng)
{
    const auto n = std::ranges::distance(first, last);
    if (n <= 0) {
        return;
    }

    auto value = Value_type<I>{};
    auto current = first;
    for (Difference_type<I> i = 0; i < n; ++i, ++current) {
        *current = value;
        ++value;
    }
    std::shuffle(first, first + n, std::forward<URBG>(rng));
}

template <std::random_access_iterator I, std::sentinel_for<I> S>
    requires std::incrementable<Value_type<I>>
void random_iota(I first, S last)
{
    random_iota(first, last, default_rng());
}

template <std::ranges::random_access_range R, typename URBG>
    requires std::uniform_random_bit_generator<std::remove_cvref_t<URBG>> &&
             std::incrementable<std::ranges::range_value_t<R>>
void random_iota(R&& range, URBG&& rng)
{
    random_iota(std::ranges::begin(range), std::ranges::end(range), std::forward<URBG>(rng));
}

template <std::ranges::random_access_range R>
    requires std::incrementable<std::ranges::range_value_t<R>>
void random_iota(R&& range)
{
    random_iota(std::forward<R>(range), default_rng());
}

}  // namespace mt
