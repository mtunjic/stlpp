#pragma once

#include <functional>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

namespace mt {

/// Dual of `fold`: while `f(state)` yields `(value, next)`, write `value`.
/// Haskell `unfoldr`.
///
/// Example:
///   auto f = [](int n) -> std::optional<std::pair<int, int>> {
///       if (n <= 0) return std::nullopt;
///       return std::pair{n, n - 1};
///   };
///   auto v = mt::unfold(f, 3);  // {3, 2, 1}
template <typename F, typename State, std::weakly_incrementable O>
O unfold(F f, State state, O dest)
{
    while (auto step = std::invoke(f, state)) {
        *dest = std::move(step->first);
        ++dest;
        state = std::move(step->second);
    }
    return dest;
}

template <typename F, typename State>
[[nodiscard]] auto unfold(F f, State state)
{
    using Step = std::invoke_result_t<F&, State>;
    using Pair = typename Step::value_type;
    std::vector<typename Pair::first_type> out;
    unfold(std::move(f), std::move(state), std::back_inserter(out));
    return out;
}

/// Write `x, f(x), f(f(x)), ...` — `n` values.
///
/// Example:
///   auto v = mt::iterate_n(5, 1, [](int x) { return x * 2; });
///   // {1, 2, 4, 8, 16}
template <typename N, typename T, typename F, std::weakly_incrementable O>
O iterate_n(N n, T x, F f, O dest)
{
    for (N i = N{0}; i < n; ++i) {
        *dest = x;
        ++dest;
        x = std::invoke(f, std::move(x));
    }
    return dest;
}

template <typename N, typename T, typename F>
[[nodiscard]] auto iterate_n(N n, T x, F f)
{
    std::vector<T> out;
    if (n <= N{0}) {
        return out;
    }
    out.reserve(static_cast<std::size_t>(n));
    iterate_n(n, std::move(x), std::move(f), std::back_inserter(out));
    return out;
}

}  // namespace mt
