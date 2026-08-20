#pragma once

#include "concepts.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <deque>
#include <functional>
#include <list>
#include <tuple>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace mt::test {

template <typename T>
concept RegularValue = mt::Regular<T>;

template <typename T>
void check_regular_axioms(const T& x)
{
    T y{};
    EXPECT_EQ(x, x);
    y = x;
    EXPECT_EQ(y, x);
    const T copy{x};
    EXPECT_EQ(copy, x);
    EXPECT_FALSE(std::less<>{}(x, x));
}

template <typename I, typename Comp = std::less<>>
    requires std::forward_iterator<I>
bool is_sorted_range(I first, I last, Comp comp = {})
{
    return std::is_sorted(first, last, comp);
}

template <typename Op, typename T>
concept AssociativeOp =
    std::regular_invocable<Op, T, T> &&
    std::convertible_to<std::invoke_result_t<Op, T, T>, T>;

template <typename Op, typename T>
void check_associative_spot(Op op, T a, T b, T c)
{
    const T ab_c = op(op(a, b), c);
    const T a_bc = op(a, op(b, c));
    EXPECT_EQ(ab_c, a_bc);
}

using EopCoreContainers =
    ::testing::Types<std::vector<int>, std::deque<int>, std::list<int>, std::array<int, 8>>;

template <typename Container>
using ContainerValue = typename Container::value_type;

template <typename Container>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename Container>
inline constexpr bool is_std_array_v = is_std_array<Container>::value;

template <typename Container>
int effective_fill(int requested)
{
    if constexpr (is_std_array_v<Container>) {
        return std::min(requested, static_cast<int>(Container{}.size()));
    }
    return requested;
}

template <typename Container>
auto filled_end(Container& c, int requested_fill)
{
    const int n = effective_fill<Container>(requested_fill);
    if constexpr (is_std_array_v<Container>) {
        return c.begin() + n;
    }
    return c.end();
}

template <typename Container>
  requires is_std_array_v<Container>
Container make_iota_container(int n, ContainerValue<Container> start = {})
{
    Container c{};
    const int fill = std::min(n, static_cast<int>(c.size()));
    for (int i = 0; i < fill; ++i) {
        c[static_cast<std::size_t>(i)] = static_cast<ContainerValue<Container>>(start + i);
    }
    return c;
}

template <typename Container>
  requires(!is_std_array_v<Container>)
Container make_iota_container(int n, ContainerValue<Container> start = {})
{
    Container c;
    if constexpr (requires { c.resize(static_cast<std::size_t>(n)); c[0]; }) {
        c.resize(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) {
            c[static_cast<std::size_t>(i)] = static_cast<ContainerValue<Container>>(start + i);
        }
    } else if constexpr (requires {
                             c.assign(static_cast<std::size_t>(n), ContainerValue<Container>{});
                         }) {
        c.assign(static_cast<std::size_t>(n), ContainerValue<Container>{});
        int i = 0;
        for (auto& v : c) {
            v = static_cast<ContainerValue<Container>>(start + i++);
        }
    } else {
        for (int i = 0; i < n; ++i) {
            c.push_back(static_cast<ContainerValue<Container>>(start + i));
        }
    }
    return c;
}

template <typename Container>
[[nodiscard]] std::size_t container_size(const Container& c)
{
    if constexpr (requires { c.size(); }) {
        return c.size();
    } else {
        return static_cast<std::size_t>(std::distance(c.begin(), c.end()));
    }
}

}  // namespace mt::test
