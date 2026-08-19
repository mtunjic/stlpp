#pragma once

#include "concepts.h"

#include <cstdint>

namespace mt {

/// True when `n` is odd. Uses the low bit; works for any `Integer`.
///
/// Example:
///   mt::odd(3);  // true
///   mt::odd(4);  // false
template <Integer N>
[[nodiscard]] constexpr bool odd(N n) noexcept
{
    return (n & N{1}) != N{0};
}

template <Integer N>
[[nodiscard]] constexpr bool even(N n) noexcept
{
    return !odd(n);
}

/// Integer division by two via right shift (EoP). Rounds toward -inf for
/// two's-complement signed values the same way `>>` does on this platform.
///
/// Example:
///   mt::half(6);  // 3
///   mt::half(7);  // 3
template <Integer N>
[[nodiscard]] constexpr N half(N n) noexcept
{
    return n >> 1;
}

/// Most-significant byte of an unsigned integer.
///
/// Example:
///   mt::high_byte(std::uint16_t{0x1234});  // 0x12
template <UnsignedInteger N>
[[nodiscard]] constexpr std::uint8_t high_byte(N x) noexcept
{
    return static_cast<std::uint8_t>(x >> ((sizeof(N) - 1U) * 8U));
}

}  // namespace mt
