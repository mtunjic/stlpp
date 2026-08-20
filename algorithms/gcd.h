#pragma once

#include "integer.h"

#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mt {

template <Integer N>
[[nodiscard]] constexpr auto unsigned_abs(N x) noexcept
{
    if constexpr (std::unsigned_integral<N>) {
        return x;
    } else {
        using U = std::make_unsigned_t<N>;
        const U u = static_cast<U>(x);
        return x < N{0} ? static_cast<U>(0) - u : u;
    }
}

/// Binary GCD (Stein). Uses only shifts, min/max, and subtraction.
/// Magnitude is computed in unsigned arithmetic so `INT_MIN` is not negated.
///
/// Example:
///   mt::stein_gcd(48, 18);  // 6
///   mt::stein_gcd(0, 7);    // 7
template <Integer N>
[[nodiscard]] constexpr N stein_gcd(N a, N b)
{
    using U = std::conditional_t<std::unsigned_integral<N>, N, std::make_unsigned_t<N>>;
    U ua = unsigned_abs(a);
    U ub = unsigned_abs(b);
    if (ua == U{0}) {
        return static_cast<N>(ub);
    }
    if (ub == U{0}) {
        return static_cast<N>(ua);
    }

    U shift = 0;
    while (!odd(ua) && !odd(ub)) {
        ua = half(ua);
        ub = half(ub);
        ++shift;
    }
    while (!odd(ua)) {
        ua = half(ua);
    }
    do {
        while (!odd(ub)) {
            ub = half(ub);
        }
        if (ua > ub) {
            std::swap(ua, ub);
        }
        ub -= ua;
    } while (ub != U{0});
    const U g = ua << shift;
    return static_cast<N>(g);
}

/// Extended Euclidean algorithm: returns `{g, x, y}` with `a*x + b*y == g`.
/// Division uses a wider signed type when available so `INT_MIN / -1` is avoided.
///
/// Example:
///   auto [g, x, y] = mt::extended_gcd(30, 12);
///   // g == 6, 30*x + 12*y == 6
template <Integer N>
[[nodiscard]] constexpr std::tuple<N, N, N> extended_gcd(N a, N b)
{
    if constexpr (std::signed_integral<N>) {
        // |b| == 1: a % b is 0, but a / -1 is UB for the most-negative value.
        if (b == N{1}) {
            return {N{1}, N{0}, N{1}};
        }
        if (b == N{-1}) {
            return {N{1}, N{0}, N{-1}};
        }
    }
    if (b == N{0}) {
        return {a, N{1}, N{0}};
    }

    using W = std::conditional_t<(sizeof(N) < sizeof(long long)), long long, N>;
    const W aa = static_cast<W>(a);
    const W bb = static_cast<W>(b);
    if constexpr (std::signed_integral<W>) {
        if (bb == W{-1}) {
            return {N{1}, N{0}, N{-1}};
        }
        if (bb == W{1}) {
            return {N{1}, N{0}, N{1}};
        }
    }
    const W q = aa / bb;
    const W r = aa % bb;
    const auto [g, x1, y1] = extended_gcd(static_cast<N>(bb), static_cast<N>(r));
    const N x = y1;
    const N y = static_cast<N>(static_cast<W>(x1) - q * static_cast<W>(y1));
    return {g, x, y};
}

}  // namespace mt
