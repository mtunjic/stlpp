#pragma once

#include "btree.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>

namespace mt {

enum class visit { pre, in, post };

template <typename C>
[[nodiscard]] std::size_t weight_recursive(C c)
{
    if (empty(c)) {
        return 0;
    }
    std::size_t l = 0;
    std::size_t r = 0;
    if (has_left_successor(c)) {
        l = weight_recursive(left_successor(c));
    }
    if (has_right_successor(c)) {
        r = weight_recursive(right_successor(c));
    }
    return l + r + 1;
}

template <typename C>
[[nodiscard]] std::size_t height_recursive(C c)
{
    if (empty(c)) {
        return 0;
    }
    std::size_t l = 0;
    std::size_t r = 0;
    if (has_left_successor(c)) {
        l = height_recursive(left_successor(c));
    }
    if (has_right_successor(c)) {
        r = height_recursive(right_successor(c));
    }
    return std::max(l, r) + 1;
}

/// Pre/in/post recursive walk. `proc(visit, coord)` is invoked at each phase.
template <typename C, typename Proc>
void traverse_nonempty_ref(C c, Proc& proc)
{
    proc(visit::pre, c);
    if (has_left_successor(c)) {
        traverse_nonempty_ref(left_successor(c), proc);
    }
    proc(visit::in, c);
    if (has_right_successor(c)) {
        traverse_nonempty_ref(right_successor(c), proc);
    }
    proc(visit::post, c);
}

template <typename C, typename Proc>
Proc traverse_nonempty(C c, Proc proc)
{
    traverse_nonempty_ref(c, proc);
    return proc;
}

/// One iterative traverse step (EoP). Returns +1 descend, 0 stay, -1 ascend.
template <typename C>
int traverse_step(visit& v, C& c)
{
    switch (v) {
    case visit::pre:
        if (has_left_successor(c)) {
            c = left_successor(c);
            return 1;
        }
        v = visit::in;
        return 0;
    case visit::in:
        if (has_right_successor(c)) {
            v = visit::pre;
            c = right_successor(c);
            return 1;
        }
        v = visit::post;
        return 0;
    case visit::post:
        if (is_left_successor(c)) {
            v = visit::in;
        }
        c = predecessor(c);
        return -1;
    }
    return 0;
}

template <typename C>
[[nodiscard]] bool reachable(C x, C y)
{
    if (empty(x)) {
        return false;
    }
    const C root = x;
    visit v = visit::pre;
    do {
        if (x == y) {
            return true;
        }
        (void)traverse_step(v, x);
    } while (!(x == root && v == visit::post));
    return false;
}

template <typename C>
[[nodiscard]] std::size_t weight(C c)
{
    if (empty(c)) {
        return 0;
    }
    const C root = c;
    visit v = visit::pre;
    std::size_t n = 1;
    do {
        (void)traverse_step(v, c);
        if (v == visit::pre) {
            ++n;
        }
    } while (!(c == root && v == visit::post));
    return n;
}

template <typename C>
[[nodiscard]] std::size_t height(C c)
{
    if (empty(c)) {
        return 0;
    }
    const C root = c;
    visit v = visit::pre;
    std::size_t n = 1;
    std::size_t m = 1;
    do {
        m = (m - 1) + static_cast<std::size_t>(traverse_step(v, c) + 1);
        n = std::max(n, m);
    } while (!(c == root && v == visit::post));
    return n;
}

template <typename C, typename Proc>
Proc traverse(C c, Proc proc)
{
    if (empty(c)) {
        return proc;
    }
    const C root = c;
    visit v = visit::pre;
    proc(visit::pre, c);
    do {
        (void)traverse_step(v, c);
        proc(v, c);
    } while (!(c == root && v == visit::post));
    return proc;
}

template <typename C0, typename C1>
[[nodiscard]] bool bifurcate_isomorphic_nonempty(C0 c0, C1 c1)
{
    if (has_left_successor(c0)) {
        if (!has_left_successor(c1) ||
            !bifurcate_isomorphic_nonempty(left_successor(c0), left_successor(c1))) {
            return false;
        }
    } else if (has_left_successor(c1)) {
        return false;
    }
    if (has_right_successor(c0)) {
        if (!has_right_successor(c1) ||
            !bifurcate_isomorphic_nonempty(right_successor(c0), right_successor(c1))) {
            return false;
        }
    } else if (has_right_successor(c1)) {
        return false;
    }
    return true;
}

template <typename C0, typename C1>
[[nodiscard]] bool bifurcate_isomorphic(C0 c0, C1 c1)
{
    if (empty(c0)) {
        return empty(c1);
    }
    if (empty(c1)) {
        return false;
    }
    const C0 root0 = c0;
    visit v0 = visit::pre;
    visit v1 = visit::pre;
    while (true) {
        (void)traverse_step(v0, c0);
        (void)traverse_step(v1, c1);
        if (v0 != v1) {
            return false;
        }
        if (c0 == root0 && v0 == visit::post) {
            return true;
        }
    }
}

template <typename C0, typename C1, typename R>
[[nodiscard]] bool bifurcate_equivalent_nonempty(C0 c0, C1 c1, R r)
{
    if (!std::invoke(r, source(c0), source(c1))) {
        return false;
    }
    if (has_left_successor(c0)) {
        if (!has_left_successor(c1) ||
            !bifurcate_equivalent_nonempty(left_successor(c0), left_successor(c1), r)) {
            return false;
        }
    } else if (has_left_successor(c1)) {
        return false;
    }
    if (has_right_successor(c0)) {
        if (!has_right_successor(c1) ||
            !bifurcate_equivalent_nonempty(right_successor(c0), right_successor(c1), r)) {
            return false;
        }
    } else if (has_right_successor(c1)) {
        return false;
    }
    return true;
}

template <typename C0, typename C1>
[[nodiscard]] bool bifurcate_equal_nonempty(C0 c0, C1 c1)
{
    return bifurcate_equivalent_nonempty(c0, c1, std::equal_to<>{});
}

template <typename C0, typename C1, typename R>
[[nodiscard]] bool bifurcate_equivalent(C0 c0, C1 c1, R r)
{
    if (empty(c0)) {
        return empty(c1);
    }
    if (empty(c1)) {
        return false;
    }
    const C0 root0 = c0;
    visit v0 = visit::pre;
    visit v1 = visit::pre;
    while (true) {
        if (v0 == visit::pre && !std::invoke(r, source(c0), source(c1))) {
            return false;
        }
        (void)traverse_step(v0, c0);
        (void)traverse_step(v1, c1);
        if (v0 != v1) {
            return false;
        }
        if (c0 == root0 && v0 == visit::post) {
            return true;
        }
    }
}

template <typename C0, typename C1>
[[nodiscard]] bool bifurcate_equal(C0 c0, C1 c1)
{
    return bifurcate_equivalent(c0, c1, std::equal_to<>{});
}

template <typename C0, typename C1, typename R>
[[nodiscard]] bool bifurcate_compare(C0 c0, C1 c1, R r)
{
    if (empty(c1)) {
        return false;
    }
    if (empty(c0)) {
        return true;
    }
    const C0 root0 = c0;
    visit v0 = visit::pre;
    visit v1 = visit::pre;
    while (true) {
        if (v0 == visit::pre) {
            if (std::invoke(r, source(c0), source(c1))) {
                return true;
            }
            if (std::invoke(r, source(c1), source(c0))) {
                return false;
            }
        }
        (void)traverse_step(v0, c0);
        (void)traverse_step(v1, c1);
        if (v0 != v1) {
            return v0 > v1;
        }
        if (c0 == root0 && v0 == visit::post) {
            return false;
        }
    }
}

template <typename C0, typename C1>
[[nodiscard]] bool bifurcate_less(C0 c0, C1 c1)
{
    return bifurcate_compare(c0, c1, std::less<>{});
}

/// Constant-space tree rotation used by `traverse_rotating` (EoP ch. 8).
template <typename C>
void tree_rotate(C& curr, C& prev)
{
    C tmp = left_successor(curr);
    set_left_successor(curr, right_successor(curr));
    set_right_successor(curr, prev);
    if (empty(tmp)) {
        prev = tmp;
        return;
    }
    prev = curr;
    curr = tmp;
}

template <typename C, typename Proc>
Proc traverse_rotating(C c, Proc proc)
{
    if (empty(c)) {
        return proc;
    }
    C curr = c;
    C prev{};
    do {
        proc(curr);
        tree_rotate(curr, prev);
    } while (!(curr == c));
    do {
        proc(curr);
        tree_rotate(curr, prev);
    } while (!(curr == c));
    proc(curr);
    tree_rotate(curr, prev);
    return proc;
}

template <typename C>
[[nodiscard]] std::size_t weight_rotating(C c)
{
    std::size_t n = 0;
    (void)traverse_rotating(c, [&](C) { ++n; });
    return n / 3;
}

template <typename N, typename Proc>
struct phased_applicator {
    N period;
    N phase;
    N n;
    Proc proc;

    void operator()(auto x)
    {
        if (n == phase) {
            proc(x);
        }
        ++n;
        if (n == period) {
            n = 0;
        }
    }
};

template <typename C, typename Proc>
Proc traverse_phased_rotating(C c, int phase, Proc proc)
{
    phased_applicator<int, Proc> applicator{3, phase, 0, std::move(proc)};
    return traverse_rotating(c, std::move(applicator)).proc;
}

}  // namespace mt
