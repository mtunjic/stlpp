#pragma once

#include "integer.h"

#include <tuple>
#include <utility>

namespace mt {

/// Binary GCD (Stein). Uses only shifts, min/max, and subtraction.
///
/// Example:
///   mt::stein_gcd(48, 18);  // 6
///   mt::stein_gcd(0, 7);    // 7
template <Integer N>
[[nodiscard]] constexpr N stein_gcd(N a, N b)
{
    auto abs_n = [](N x) -> N {
        return x < N{0} ? static_cast<N>(-x) : x;
    };
    a = abs_n(a);
    b = abs_n(b);
    if (a == N{0}) {
        return b;
    }
    if (b == N{0}) {
        return a;
    }

    N shift = 0;
    while (!odd(a) && !odd(b)) {
        a = half(a);
        b = half(b);
        ++shift;
    }
    while (!odd(a)) {
        a = half(a);
    }
    do {
        while (!odd(b)) {
            b = half(b);
        }
        if (a > b) {
            std::swap(a, b);
        }
        b -= a;
    } while (b != N{0});
    return a << shift;
}

/// Extended Euclidean algorithm: returns `{g, x, y}` with `a*x + b*y == g`.
///
/// Example:
///   auto [g, x, y] = mt::extended_gcd(30, 12);
///   // g == 6, 30*x + 12*y == 6
template <Integer N>
[[nodiscard]] constexpr std::tuple<N, N, N> extended_gcd(N a, N b)
{
    if (b == N{0}) {
        return {a, N{1}, N{0}};
    }
    const auto [g, x1, y1] = extended_gcd(b, static_cast<N>(a % b));
    const N x = y1;
    const N y = static_cast<N>(x1 - (a / b) * y1);
    return {g, x, y};
}

}  // namespace mt
