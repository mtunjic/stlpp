#pragma once

#include <functional>
#include <utility>

namespace mt {

namespace detail {

template <bool Strict, typename R>
struct compare_strict_or_reflexive;

template <typename R>
struct compare_strict_or_reflexive<true, R> {
    template <typename T>
    constexpr bool operator()(const T& a, const T& b, R r) const
    {
        return std::invoke(r, a, b);
    }
};

template <typename R>
struct compare_strict_or_reflexive<false, R> {
    template <typename T>
    constexpr bool operator()(const T& a, const T& b, R r) const
    {
        return !std::invoke(r, b, a);
    }
};

}  // namespace detail

/// Min of two under relation `r` (EoP `select_0_2`).
template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& select_0_2(const T& a, const T& b, R r = {})
{
    if (std::invoke(r, b, a)) {
        return b;
    }
    return a;
}

/// Max of two under relation `r` (EoP `select_1_2`).
template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& select_1_2(const T& a, const T& b, R r = {})
{
    if (std::invoke(r, b, a)) {
        return a;
    }
    return b;
}

template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& select_0_3(const T& a, const T& b, const T& c, R r = {})
{
    return select_0_2(select_0_2(a, b, r), c, r);
}

template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& select_2_3(const T& a, const T& b, const T& c, R r = {})
{
    return select_1_2(select_1_2(a, b, r), c, r);
}

template <typename T, typename R>
[[nodiscard]] constexpr const T& select_1_3_ab(const T& a, const T& b, const T& c, R r)
{
    if (!std::invoke(r, c, b)) {
        return b;
    }
    return select_1_2(a, c, r);
}

/// Median of three (EoP `select_1_3`).
template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& select_1_3(const T& a, const T& b, const T& c, R r = {})
{
    if (std::invoke(r, b, a)) {
        return select_1_3_ab(b, a, c, r);
    }
    return select_1_3_ab(a, b, c, r);
}

template <typename T, typename R>
[[nodiscard]] constexpr const T& select_1_4_ab_cd(const T& a, const T& b, const T& c, const T& d,
                                                   R r)
{
    if (std::invoke(r, c, a)) {
        return select_0_2(a, d, r);
    }
    return select_0_2(b, c, r);
}

template <typename T, typename R>
[[nodiscard]] constexpr const T& select_1_4_ab(const T& a, const T& b, const T& c, const T& d, R r)
{
    if (std::invoke(r, d, c)) {
        return select_1_4_ab_cd(a, b, d, c, r);
    }
    return select_1_4_ab_cd(a, b, c, d, r);
}

/// Second-smallest of four (EoP `select_1_4`).
template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& select_1_4(const T& a, const T& b, const T& c, const T& d,
                                            R r = {})
{
    if (std::invoke(r, b, a)) {
        return select_1_4_ab(b, a, c, d, r);
    }
    return select_1_4_ab(a, b, c, d, r);
}

// Stability-indexed helpers (needed for stable median_5).

template <int Ia, int Ib, typename T, typename R>
[[nodiscard]] constexpr const T& select_0_2_stable(const T& a, const T& b, R r)
{
    detail::compare_strict_or_reflexive<(Ia < Ib), R> cmp;
    if (cmp(b, a, r)) {
        return b;
    }
    return a;
}

template <int Ia, int Ib, typename T, typename R>
[[nodiscard]] constexpr const T& select_1_2_stable(const T& a, const T& b, R r)
{
    detail::compare_strict_or_reflexive<(Ia < Ib), R> cmp;
    if (cmp(b, a, r)) {
        return a;
    }
    return b;
}

template <int Ia, int Ib, int Ic, int Id, typename T, typename R>
[[nodiscard]] constexpr const T& select_1_4_ab_cd_stable(const T& a, const T& b, const T& c,
                                                          const T& d, R r)
{
    detail::compare_strict_or_reflexive<(Ia < Ic), R> cmp;
    if (cmp(c, a, r)) {
        return select_0_2_stable<Ia, Id>(a, d, r);
    }
    return select_0_2_stable<Ib, Ic>(b, c, r);
}

template <int Ia, int Ib, int Ic, int Id, typename T, typename R>
[[nodiscard]] constexpr const T& select_1_4_ab_stable(const T& a, const T& b, const T& c,
                                                       const T& d, R r)
{
    detail::compare_strict_or_reflexive<(Ic < Id), R> cmp;
    if (cmp(d, c, r)) {
        return select_1_4_ab_cd_stable<Ia, Ib, Id, Ic>(a, b, d, c, r);
    }
    return select_1_4_ab_cd_stable<Ia, Ib, Ic, Id>(a, b, c, d, r);
}

template <int Ia, int Ib, int Ic, int Id, typename T, typename R>
[[nodiscard]] constexpr const T& select_1_4_stable(const T& a, const T& b, const T& c, const T& d,
                                                    R r)
{
    detail::compare_strict_or_reflexive<(Ia < Ib), R> cmp;
    if (cmp(b, a, r)) {
        return select_1_4_ab_stable<Ib, Ia, Ic, Id>(b, a, c, d, r);
    }
    return select_1_4_ab_stable<Ia, Ib, Ic, Id>(a, b, c, d, r);
}

template <int Ia, int Ib, int Ic, int Id, int Ie, typename T, typename R>
[[nodiscard]] constexpr const T& select_2_5_ab_cd(const T& a, const T& b, const T& c, const T& d,
                                                   const T& e, R r)
{
    detail::compare_strict_or_reflexive<(Ia < Ic), R> cmp;
    if (cmp(c, a, r)) {
        return select_1_4_ab_stable<Ia, Ib, Id, Ie>(a, b, d, e, r);
    }
    return select_1_4_ab_stable<Ic, Id, Ib, Ie>(c, d, b, e, r);
}

template <int Ia, int Ib, int Ic, int Id, int Ie, typename T, typename R>
[[nodiscard]] constexpr const T& select_2_5_ab(const T& a, const T& b, const T& c, const T& d,
                                                const T& e, R r)
{
    detail::compare_strict_or_reflexive<(Ic < Id), R> cmp;
    if (cmp(d, c, r)) {
        return select_2_5_ab_cd<Ia, Ib, Id, Ic, Ie>(a, b, d, c, e, r);
    }
    return select_2_5_ab_cd<Ia, Ib, Ic, Id, Ie>(a, b, c, d, e, r);
}

template <int Ia, int Ib, int Ic, int Id, int Ie, typename T, typename R>
[[nodiscard]] constexpr const T& select_2_5(const T& a, const T& b, const T& c, const T& d,
                                             const T& e, R r)
{
    detail::compare_strict_or_reflexive<(Ia < Ib), R> cmp;
    if (cmp(b, a, r)) {
        return select_2_5_ab<Ib, Ia, Ic, Id, Ie>(b, a, c, d, e, r);
    }
    return select_2_5_ab<Ia, Ib, Ic, Id, Ie>(a, b, c, d, e, r);
}

/// Stable median of five (EoP `median_5` / `select_2_5`).
template <typename T, typename R = std::less<>>
[[nodiscard]] constexpr const T& median_5(const T& a, const T& b, const T& c, const T& d,
                                          const T& e, R r = {})
{
    return select_2_5<0, 1, 2, 3, 4>(a, b, c, d, e, r);
}

}  // namespace mt
