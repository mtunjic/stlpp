#pragma once

#include <functional>
#include <utility>

namespace mt {

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

}  // namespace mt
