#pragma once

#include "concepts.h"

#include <cctype>
#include <cmath>
#include <cstring>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mt {

/// Modular product: `(n * m) % modulus` with a wider intermediate product.
///
/// Example:
///   mt::modulo_multiply<int> op{5};
///   op(3, 4);  // 12 % 5 == 2
template <Integer I>
struct modulo_multiply {
    I modulus{};
    explicit modulo_multiply(I m) : modulus(std::move(m)) {}
    I operator()(const I& n, const I& m) const
    {
        if (modulus == I{0}) {
            throw std::domain_error("modulo_multiply: zero modulus");
        }
        using U = std::conditional_t<std::unsigned_integral<I>, I, std::make_unsigned_t<I>>;
        const U mod = [&] {
            if constexpr (std::unsigned_integral<I>) {
                return static_cast<U>(modulus);
            } else {
                const U u = static_cast<U>(modulus);
                return modulus < I{0} ? static_cast<U>(0) - u : u;
            }
        }();
        auto reduce = [mod](I x) -> U {
            if constexpr (std::unsigned_integral<I>) {
                return static_cast<U>(x % static_cast<I>(mod));
            } else {
                I r = static_cast<I>(x % static_cast<I>(mod));
                if (r < I{0}) {
                    r += static_cast<I>(mod);
                }
                return static_cast<U>(r);
            }
        };
        auto addmod = [mod](U x, U y) -> U {
            if (x >= mod - y) {
                return x - (mod - y);
            }
            return x + y;
        };
        U a = reduce(n);
        U b = reduce(m);
        U r = 0;
        while (b != U{0}) {
            if ((b & U{1}) != U{0}) {
                r = addmod(r, a);
            }
            a = addmod(a, a);
            b >>= 1;
        }
        return static_cast<I>(r);
    }
};

template <Integer I>
[[nodiscard]] constexpr I identity_element(const modulo_multiply<I>&)
{
    return I{1};
}

template <Number N>
[[nodiscard]] constexpr bool is_zero(const N& x)
{
    return x == N{0};
}

template <Number N>
[[nodiscard]] constexpr bool is_positive(const N& x)
{
    return x > N{0};
}

/// Arithmetic right-shift by one. Precondition: `x > 0`.
///
/// Example:
///   int n = 10;
///   mt::halve_non_negative(n);  // 5
template <Integer I>
constexpr void halve_non_negative(I& x)
{
    x >>= 1;
}

template <Integer I>
constexpr void double_non_negative(I& x)
{
    x <<= 1;
}

template <Integer I>
[[nodiscard]] constexpr bool is_odd(const I& x)
{
    return (x & I{1}) != I{0};
}

template <Integer I>
[[nodiscard]] constexpr bool is_even(const I& x)
{
    return !is_odd(x);
}

template <typename N>
struct plus {
    constexpr N operator()(const N& x, const N& y) const { return x + y; }
};

template <typename N>
struct multiplies {
    constexpr N operator()(const N& x, const N& y) const { return x * y; }
};

/// Division that returns `x` when `y == 0` (old STL++ convention).
template <Number N>
struct divides {
    constexpr N operator()(const N& x, const N& y) const { return y == N{0} ? x : x / y; }
};

template <TotallyOrdered T>
struct minimum {
    constexpr T operator()(const T& x, const T& y) const { return y < x ? y : x; }
};

/// Squared Euclidean norm of a pair: `x*x + y*y`.
template <Number T>
struct norm {
    constexpr T operator()(const T& x, const T& y) const { return x * x + y * y; }
};

template <Number T>
struct conversion {
    T operator()(const T& x, const T& y) const
    {
        return static_cast<T>(static_cast<double>(x) + static_cast<double>(y));
    }
};

template <Number N>
struct square_root {
    N operator()(N x, N y) const
    {
        return static_cast<N>(std::sqrt(std::abs(static_cast<double>(x + y))));
    }
};

template <Number N>
struct cube_root_pow {
    N operator()(N x, N y) const
    {
        return static_cast<N>(std::pow(std::abs(static_cast<double>(x + y)), 1.0 / 3.0));
    }
};

template <Number N>
struct root {
    N operator()(const N& x) const { return static_cast<N>(std::sqrt(static_cast<double>(x))); }
};

template <Number N>
struct volume {
    constexpr N operator()(N x, N y, N z) const { return x * y * z; }
};

/// Unary predicate `y < bound`.
///
/// Example:
///   mt::keep_if(v, mt::less_than<int>{8});
template <TotallyOrdered T>
struct less_than {
    T bound{};
    explicit less_than(T x) : bound(std::move(x)) {}
    constexpr bool operator()(const T& y) const { return y < bound; }
};

template <TotallyOrdered T>
struct greater_than {
    T bound{};
    explicit greater_than(T x) : bound(std::move(x)) {}
    constexpr bool operator()(const T& y) const { return y > bound; }
};

struct to_lower_char {
    char operator()(char c) const
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
};

struct to_upper_char {
    char operator()(char c) const
    {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
};

struct non_letter {
    bool operator()(char c) const { return std::isalpha(static_cast<unsigned char>(c)) == 0; }
};

struct digit {
    bool operator()(char c) const { return std::isdigit(static_cast<unsigned char>(c)) != 0; }
};

/// True for ASCII vowels (both cases).
///
/// Example:
///   mt::vowel{}('e');  // true
///   mt::vowel{}('s');  // false
struct vowel {
    bool operator()(char c) const { return std::strchr("aeiouAEIOU", c) != nullptr; }
};

template <typename T>
struct printer {
    std::ostream* out = &std::cout;
    void operator()(const T& s) const { *out << s << '\n'; }
};

}  // namespace mt
