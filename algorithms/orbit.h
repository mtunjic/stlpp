#pragma once

#include "concepts.h"

#include <functional>
#include <utility>

namespace mt {

/// Apply transformation `f` exactly `n` times (EoP `power_unary`).
template <typename T, Integer N, typename F>
[[nodiscard]] T power_unary(T x, N n, F f)
{
    while (n != N{0}) {
        --n;
        x = std::invoke(f, std::move(x));
    }
    return x;
}

/// Steps from `x` to `y` under `f`. Precondition: `y` is reachable from `x`.
template <typename T, typename F>
[[nodiscard]] std::size_t transformation_distance(T x, const T& y, F f)
{
    std::size_t n = 0;
    while (!(x == y)) {
        x = std::invoke(f, std::move(x));
        ++n;
    }
    return n;
}

/// Floyd collision on a total transformation. Precondition: the orbit of `x`
/// is finite and circular (otherwise this does not terminate).
///
/// Example:
///   auto f = [](int x) { return (x + 1) % 5; };
///   mt::collision_point(0, f);  // some point on the cycle
template <typename T, typename F>
[[nodiscard]] T collision_point(T x, F f)
{
    T slow = x;
    T fast = std::invoke(f, x);
    while (fast != slow) {
        slow = std::invoke(f, std::move(slow));
        fast = std::invoke(f, fast);
        fast = std::invoke(f, std::move(fast));
    }
    return slow;
}

/// Safe collision: `p(y)` means `f(y)` is defined. If the orbit terminates,
/// returns a point where `p` is false (or the start if `!p(x)`).
///
/// Example:
///   auto f = [](int x) { return x + 1; };
///   auto p = [](int x) { return x < 10; };
///   mt::collision_point(0, f, p);  // 10 (first undefined)
template <typename T, typename F, typename P>
[[nodiscard]] T collision_point(T x, F f, P p)
{
    if (!std::invoke(p, x)) {
        return x;
    }
    T slow = x;
    T fast = std::invoke(f, x);
    while (fast != slow) {
        slow = std::invoke(f, std::move(slow));
        if (!std::invoke(p, fast)) {
            return fast;
        }
        fast = std::invoke(f, std::move(fast));
        if (!std::invoke(p, fast)) {
            return fast;
        }
        fast = std::invoke(f, std::move(fast));
    }
    return fast;
}

/// True when the guarded orbit ends (collision point fails `p`).
template <typename T, typename F, typename P>
[[nodiscard]] bool terminating(const T& x, F f, P p)
{
    return !std::invoke(p, collision_point(x, f, p));
}

/// True when `x` lies on the cycle of a nonterminating orbit.
template <typename T, typename F>
[[nodiscard]] bool circular(const T& x, F f)
{
    return x == std::invoke(f, collision_point(x, f));
}

/// True when guarded orbit is circular: collision still defined and `x == f(y)`.
template <typename T, typename F, typename P>
[[nodiscard]] bool circular(const T& x, F f, P p)
{
    const T y = collision_point(x, f, p);
    return std::invoke(p, y) && x == std::invoke(f, y);
}

/// Walk `x0` and `x1` in lockstep until equal (EoP `convergent_point`).
template <typename T, typename F>
[[nodiscard]] T convergent_point(T x0, T x1, F f)
{
    while (!(x0 == x1)) {
        x0 = std::invoke(f, std::move(x0));
        x1 = std::invoke(f, std::move(x1));
    }
    return x0;
}

/// First point of the cycle for a nonterminating orbit (rho shape).
template <typename T, typename F>
[[nodiscard]] T connection_point(const T& x, F f)
{
    return convergent_point(x, std::invoke(f, collision_point(x, f)), f);
}

/// Connection point for a guarded orbit; terminal if the orbit ends.
template <typename T, typename F, typename P>
[[nodiscard]] T connection_point(const T& x, F f, P p)
{
    const T y = collision_point(x, f, p);
    if (!std::invoke(p, y)) {
        return y;
    }
    return convergent_point(x, std::invoke(f, y), f);
}

}  // namespace mt
