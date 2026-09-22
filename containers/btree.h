#pragma once

#include <concepts>
#include <cstddef>
#include <deque>
#include <unordered_map>
#include <utility>

namespace mt {

template <typename T>
struct btree_node {
    T value{};
    btree_node* left = nullptr;
    btree_node* right = nullptr;
    btree_node* parent = nullptr;
};

/// Bifurcate coordinate: pointer to a binary-tree node, or empty (`nullptr`).
template <typename T>
class btree_coord {
    btree_node<T>* p_ = nullptr;

public:
    btree_coord() = default;
    explicit btree_coord(btree_node<T>* p) : p_(p) {}

    [[nodiscard]] btree_node<T>* ptr() const { return p_; }

    friend bool operator==(btree_coord a, btree_coord b) { return a.p_ == b.p_; }
};

template <typename T>
[[nodiscard]] constexpr bool empty(btree_coord<T> c)
{
    return c.ptr() == nullptr;
}

template <typename T>
[[nodiscard]] btree_coord<T> left_successor(btree_coord<T> c)
{
    return btree_coord<T>{c.ptr()->left};
}

template <typename T>
[[nodiscard]] btree_coord<T> right_successor(btree_coord<T> c)
{
    return btree_coord<T>{c.ptr()->right};
}

template <typename T>
[[nodiscard]] bool has_left_successor(btree_coord<T> c)
{
    return !empty(left_successor(c));
}

template <typename T>
[[nodiscard]] bool has_right_successor(btree_coord<T> c)
{
    return !empty(right_successor(c));
}

template <typename T>
[[nodiscard]] const T& source(btree_coord<T> c)
{
    return c.ptr()->value;
}

template <typename T>
[[nodiscard]] T& sink(btree_coord<T> c)
{
    return c.ptr()->value;
}

template <typename T>
void set_left_successor(btree_coord<T> c, btree_coord<T> child)
{
    c.ptr()->left = child.ptr();
    if (child.ptr() != nullptr) {
        child.ptr()->parent = c.ptr();
    }
}

template <typename T>
void set_right_successor(btree_coord<T> c, btree_coord<T> child)
{
    c.ptr()->right = child.ptr();
    if (child.ptr() != nullptr) {
        child.ptr()->parent = c.ptr();
    }
}

template <typename T>
[[nodiscard]] bool has_predecessor(btree_coord<T> c)
{
    return c.ptr() != nullptr && c.ptr()->parent != nullptr;
}

template <typename T>
[[nodiscard]] btree_coord<T> predecessor(btree_coord<T> c)
{
    return btree_coord<T>{c.ptr()->parent};
}

template <typename T>
[[nodiscard]] bool is_left_successor(btree_coord<T> j)
{
    const auto i = predecessor(j);
    return has_left_successor(i) && left_successor(i) == j;
}

template <typename T>
[[nodiscard]] bool is_right_successor(btree_coord<T> j)
{
    const auto i = predecessor(j);
    return has_right_successor(i) && right_successor(i) == j;
}

/// Owning binary tree with stable node addresses (deque). Coordinates remain
/// valid until the tree is destroyed or a node is discarded with the tree.
template <typename T>
class btree {
    std::deque<btree_node<T>> nodes_;
    btree_node<T>* root_ = nullptr;

public:
    using coord = btree_coord<T>;

    btree() = default;

    btree(const btree& other)
        requires std::copy_constructible<T>
        : nodes_(other.nodes_)
    {
        std::unordered_map<const btree_node<T>*, btree_node<T>*> copies;
        for (std::size_t i = 0; i < nodes_.size(); ++i) {
            copies.emplace(&other.nodes_[i], &nodes_[i]);
        }
        auto rebind = [&](btree_node<T>* p) { return p ? copies.at(p) : nullptr; };
        for (auto& n : nodes_) {
            n.left = rebind(n.left);
            n.right = rebind(n.right);
            n.parent = rebind(n.parent);
        }
        root_ = rebind(other.root_);
    }

    btree(btree&& other) : btree() { swap(other); }

    btree& operator=(btree other)
    {
        swap(other);
        return *this;
    }

    void swap(btree& other) noexcept
    {
        nodes_.swap(other.nodes_);
        std::swap(root_, other.root_);
    }

    [[nodiscard]] coord root() const { return coord{root_}; }

    coord set_root(T value)
    {
        nodes_.push_back(btree_node<T>{std::move(value)});
        root_ = &nodes_.back();
        return coord{root_};
    }

    coord add_left(coord parent, T value)
    {
        nodes_.push_back(btree_node<T>{std::move(value)});
        auto* n = &nodes_.back();
        n->parent = parent.ptr();
        set_left_successor(parent, coord{n});
        return coord{n};
    }

    coord add_right(coord parent, T value)
    {
        nodes_.push_back(btree_node<T>{std::move(value)});
        auto* n = &nodes_.back();
        n->parent = parent.ptr();
        set_right_successor(parent, coord{n});
        return coord{n};
    }

    [[nodiscard]] std::size_t size() const { return nodes_.size(); }
};

}  // namespace mt
