#pragma once

#include "concepts.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace mt {

// binary_counter (Stepanov)
//
// Flow: a sequence of slots, each either `zero` (empty) or a value. Adding
// carry walks from the least-significant slot:
//   1. empty slot  -> store carry, absorb, return zero
//   2. occupied    -> carry = op(slot, carry), write zero, continue
//   3. past last   -> leftover carry is overflow (caller must grow)
// Reduce skips leading zeros and folds remaining non-zero slots with `op`.
//
// Precondition: `op` is associative; `zero` is an identity for `op` (or a
// sentinel that is never a stored value).
// Iterator strength: forward (add mutates in place); input (reduce).
// Complexity: add is amortized O(1) combinations per call (binary carry);
// reduce is O(number of slots) = O(log n) after n additions.
//
// Example:
//   mt::binary_counter<int, std::plus<int>> c(std::plus<int>{}, 0);
//   for (int x : {1, 2, 3, 4}) c.add(x);
//   c.reduce();  // 10

template <Forward_iterator I, Binary_operation<Value_type<I>> Op>
  requires Regular<Value_type<I>>
[[nodiscard]] constexpr Value_type<I> add_to_counter(
    I first, I last, Op op, Value_type<I> carry, const Value_type<I>& zero)
{
    while (first != last) {
        if (*first == zero) {
            *first = std::move(carry);
            return zero;
        }
        carry = op(std::move(*first), std::move(carry));
        *first = zero;
        ++first;
    }
    return carry;
}

template <Input_iterator I, Binary_operation<Value_type<I>> Op>
  requires Regular<Value_type<I>>
[[nodiscard]] constexpr Value_type<I> reduce_counter(
    I first, I last, Op op, const Value_type<I>& zero)
{
    while (first != last && *first == zero) {
        ++first;
    }
    if (first == last) {
        return zero;
    }

    auto result = *first;
    ++first;
    while (first != last) {
        if (*first != zero) {
            result = op(std::move(result), *first);
        }
        ++first;
    }
    return result;
}

template <Regular T, Binary_operation<T> Op>
class binary_counter {
public:
    binary_counter(Op op, T zero)
        : op_(std::move(op)), zero_(std::move(zero)) {}

    void add(T value) {
        value = add_to_counter(digits_.begin(), digits_.end(), op_, std::move(value), zero_);
        if (value != zero_) {
            digits_.push_back(std::move(value));
        }
    }

    [[nodiscard]] T reduce() const {
        return reduce_counter(digits_.cbegin(), digits_.cend(), op_, zero_);
    }

    [[nodiscard]] bool empty() const noexcept { return digits_.empty(); }

    [[nodiscard]] std::size_t size() const noexcept { return digits_.size(); }

private:
    std::vector<T> digits_{};
    Op op_;
    T zero_;
};

}  // namespace mt
