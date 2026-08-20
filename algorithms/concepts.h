#pragma once

#include <concepts>
#include <functional>
#include <iterator>
#include <type_traits>

namespace mt {

// Regular types (Dehnert & Stepanov 1998 / EoP ch. 1): copy, assign, and
// equality behave like built-ins. std::regular is the C++20 spelling.
template <typename T>
concept Semiregular = std::semiregular<T>;

template <typename T>
concept Regular = std::regular<T>;

template <typename T>
concept TotallyOrdered = std::totally_ordered<T>;

template <typename T>
concept Integer = std::integral<T>;

template <typename T>
concept Integral = std::integral<T>;

template <typename T>
concept UnsignedInteger = std::unsigned_integral<T>;

template <typename T>
concept UnsignedIntegral = std::unsigned_integral<T>;

template <typename T>
concept Number = std::is_arithmetic_v<T>;

template <typename T>
concept Character =
    std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> ||
    std::same_as<T, wchar_t> || std::same_as<T, char8_t> || std::same_as<T, char16_t> ||
    std::same_as<T, char32_t>;

template <typename I>
concept Input_iterator = std::input_iterator<I>;

template <typename I>
concept Output_iterator = std::output_iterator<I, std::iter_value_t<I>>;

template <typename I>
concept Forward_iterator = std::forward_iterator<I>;

template <typename I>
concept Bidirectional_iterator = std::bidirectional_iterator<I>;

template <typename I>
concept Random_access_iterator = std::random_access_iterator<I>;

template <typename I>
using Value_type = std::iter_value_t<I>;

template <typename I>
using Difference_type = std::iter_difference_t<I>;

template <typename Op, typename T>
concept Binary_operation =
    std::regular_invocable<Op, T, T> && std::convertible_to<std::invoke_result_t<Op, T, T>, T>;

template <typename P, typename T>
concept Unary_predicate = std::predicate<P, T>;

/// Stepanov iterator basis: a semiregular object that holds the iterator
/// state and the operations to move and dereference it.
template <typename B>
concept IteratorBasis = Semiregular<B> && requires(B& b, const B& cb) {
    typename B::state_type;
    typename B::value_type;
    typename B::reference;
    typename B::difference_type;
    typename B::iterator_category;
    { cb.state() } -> std::convertible_to<typename B::state_type>;
    { cb.deref() } -> std::convertible_to<typename B::reference>;
    b.increment();
};

template <typename B>
concept BidirectionalIteratorBasis = IteratorBasis<B> && requires(B& b) { b.decrement(); };

template <typename B>
concept RandomAccessIteratorBasis =
    BidirectionalIteratorBasis<B> &&
    requires(B& b, const B& cb, typename B::difference_type n, const B& other) {
        b.offset(n);
        { cb.difference(other) } -> std::convertible_to<typename B::difference_type>;
    };

}  // namespace mt
