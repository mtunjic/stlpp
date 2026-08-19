#pragma once

#include "integer.h"

#include <utility>

namespace mt {

/// Repeated squaring: `a ⊕ a ⊕ ... ⊕ a` (`n` times). Precondition: `n > 0`.
/// `op` must be associative. EoP / SICP `repeated`.
///
/// Example:
///   mt::power(2, 10, std::multiplies<int>{});  // 1024
///   mt::power(3, 5, std::plus<int>{});         // 15
template <typename A, Integer N, typename Op>
[[nodiscard]] A power(A a, N n, Op op)
{
    while (even(n)) {
        a = op(a, a);
        n = half(n);
    }
    if (n == N{1}) {
        return a;
    }
    A r = a;
    a = op(a, a);
    n = half(n - N{1});
    while (true) {
        if (odd(n)) {
            r = op(r, a);
            if (n == N{1}) {
                return r;
            }
        }
        a = op(a, a);
        n = half(n);
    }
}

/// Monoid power: `n == 0` returns `identity`.
///
/// Example:
///   mt::power(2, 0, std::multiplies<int>{}, 1);  // 1
template <typename A, Integer N, typename Op>
[[nodiscard]] A power(A a, N n, Op op, A identity)
{
    if (n == N{0}) {
        return identity;
    }
    return power(std::move(a), n, std::move(op));
}

}  // namespace mt
