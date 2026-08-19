#pragma once

#include "concepts.h"

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace mt {

/// Iterator generated from a Stepanov *basis* (state + increment/deref).
/// Bidirectional and random-access operations exist only when the basis
/// provides them.
///
/// Example:
///   struct counting_basis { ... increment(); deref(); };
///   mt::iterator<counting_basis> i{0};
template <IteratorBasis B>
class iterator {
public:
    using basis_type = B;
    using state_type = typename B::state_type;
    using value_type = typename B::value_type;
    using difference_type = typename B::difference_type;
    using reference = typename B::reference;
    using iterator_category = typename B::iterator_category;
    using iterator_concept = typename B::iterator_category;
    using pointer = std::conditional_t<std::is_lvalue_reference_v<reference>,
                                       std::add_pointer_t<std::remove_reference_t<reference>>, void>;

    B basis{};

    iterator() = default;
    explicit iterator(state_type x) : basis{std::move(x)} {}
    explicit iterator(B b) : basis{std::move(b)} {}

    [[nodiscard]] state_type state() const { return basis.state(); }

    [[nodiscard]] reference operator*() const { return basis.deref(); }

    pointer operator->() const
        requires std::is_lvalue_reference_v<reference>
    {
        return std::addressof(basis.deref());
    }

    iterator& operator++()
    {
        basis.increment();
        return *this;
    }

    iterator operator++(int)
    {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const iterator& x, const iterator& y)
    {
        return x.state() == y.state();
    }

    iterator& operator--()
        requires BidirectionalIteratorBasis<B>
    {
        basis.decrement();
        return *this;
    }

    iterator operator--(int)
        requires BidirectionalIteratorBasis<B>
    {
        iterator tmp = *this;
        --*this;
        return tmp;
    }

    iterator& operator+=(difference_type n)
        requires RandomAccessIteratorBasis<B>
    {
        basis.offset(n);
        return *this;
    }

    iterator& operator-=(difference_type n)
        requires RandomAccessIteratorBasis<B>
    {
        return *this += -n;
    }

    [[nodiscard]] reference operator[](difference_type n) const
        requires RandomAccessIteratorBasis<B>
    {
        return *(*this + n);
    }

    friend difference_type operator-(const iterator& x, const iterator& y)
        requires RandomAccessIteratorBasis<B>
    {
        return x.basis.difference(y.basis);
    }

    friend iterator operator+(iterator x, difference_type n)
        requires RandomAccessIteratorBasis<B>
    {
        return x += n;
    }

    friend iterator operator+(difference_type n, iterator x)
        requires RandomAccessIteratorBasis<B>
    {
        return x += n;
    }

    friend iterator operator-(iterator x, difference_type n)
        requires RandomAccessIteratorBasis<B>
    {
        return x += -n;
    }

    friend bool operator<(const iterator& x, const iterator& y)
        requires RandomAccessIteratorBasis<B>
    {
        return (x - y) < 0;
    }

    friend bool operator>(const iterator& x, const iterator& y)
        requires RandomAccessIteratorBasis<B>
    {
        return y < x;
    }

    friend bool operator<=(const iterator& x, const iterator& y)
        requires RandomAccessIteratorBasis<B>
    {
        return !(y < x);
    }

    friend bool operator>=(const iterator& x, const iterator& y)
        requires RandomAccessIteratorBasis<B>
    {
        return !(x < y);
    }
};

}  // namespace mt
