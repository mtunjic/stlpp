#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace mt {

/// Binary concatenation tree of strings. Concat is O(1); indexing is O(height).
/// Copies share structure (`shared_ptr` nodes).
///
/// Example:
///   mt::rope a{"hello "};
///   mt::rope b{"world"};
///   auto c = concat(a, b);
///   c.at(6);           // 'w'
///   c.to_string();     // "hello world"
class rope {
    struct node {
        std::size_t len = 0;
        std::string leaf;
        std::shared_ptr<node> left;
        std::shared_ptr<node> right;

        [[nodiscard]] bool is_leaf() const { return !left && !right; }
    };

    std::shared_ptr<node> root_;

    static std::shared_ptr<node> make_leaf(std::string s)
    {
        auto n = std::make_shared<node>();
        n->len = s.size();
        n->leaf = std::move(s);
        return n;
    }

    static std::shared_ptr<node> make_concat(std::shared_ptr<node> l, std::shared_ptr<node> r)
    {
        auto n = std::make_shared<node>();
        n->left = std::move(l);
        n->right = std::move(r);
        n->len = (n->left ? n->left->len : 0) + (n->right ? n->right->len : 0);
        return n;
    }

    static char at_node(const node* n, std::size_t i)
    {
        while (n && !n->is_leaf()) {
            const std::size_t left_len = n->left ? n->left->len : 0;
            if (i < left_len) {
                n = n->left.get();
            } else {
                i -= left_len;
                n = n->right.get();
            }
        }
        if (!n || i >= n->leaf.size()) {
            throw std::out_of_range("rope::at");
        }
        return n->leaf[i];
    }

    static void flatten(const node* n, std::string& out)
    {
        if (!n) {
            return;
        }
        if (n->is_leaf()) {
            out.append(n->leaf);
            return;
        }
        flatten(n->left.get(), out);
        flatten(n->right.get(), out);
    }

public:
    rope() = default;
    explicit rope(std::string s) : root_(make_leaf(std::move(s))) {}
    explicit rope(const char* s) : rope(std::string(s ? s : "")) {}

    [[nodiscard]] std::size_t size() const { return root_ ? root_->len : 0; }
    [[nodiscard]] bool empty() const { return size() == 0; }

    [[nodiscard]] char at(std::size_t i) const
    {
        if (i >= size()) {
            throw std::out_of_range("rope::at");
        }
        return at_node(root_.get(), i);
    }

    [[nodiscard]] std::string to_string() const
    {
        std::string out;
        out.reserve(size());
        flatten(root_.get(), out);
        return out;
    }

    void append(std::string s)
    {
        if (s.empty()) {
            return;
        }
        auto leaf = make_leaf(std::move(s));
        root_ = root_ ? make_concat(std::move(root_), std::move(leaf)) : std::move(leaf);
    }

    friend rope concat(rope a, rope b);
};

/// O(1) structural concat. Named `concat` like the range algorithm; overload
/// resolution picks this when both arguments are `rope`.
[[nodiscard]] inline rope concat(rope a, rope b)
{
    if (!a.root_) {
        return b;
    }
    if (!b.root_) {
        return a;
    }
    rope r;
    r.root_ = rope::make_concat(std::move(a.root_), std::move(b.root_));
    return r;
}

}  // namespace mt
