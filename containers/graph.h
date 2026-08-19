#pragma once

#include <cstddef>
#include <vector>

namespace mt {

/// Directed adjacency-list graph. Vertices are `0 .. size()-1`.
///
/// Example:
///   mt::graph g(3);
///   g.add_edge(0, 1);
///   g.add_edge(1, 2);
///   g.neighbors(0);  // {1}
class graph {
    std::vector<std::vector<std::size_t>> adj_;

public:
    graph() = default;
    explicit graph(std::size_t n) : adj_(n) {}

    std::size_t add_vertex()
    {
        adj_.emplace_back();
        return adj_.size() - 1;
    }

    void add_edge(std::size_t u, std::size_t v) { adj_[u].push_back(v); }

    void add_undirected_edge(std::size_t u, std::size_t v)
    {
        add_edge(u, v);
        add_edge(v, u);
    }

    [[nodiscard]] std::size_t size() const { return adj_.size(); }

    [[nodiscard]] const std::vector<std::size_t>& neighbors(std::size_t u) const { return adj_[u]; }
    [[nodiscard]] std::vector<std::size_t>& neighbors(std::size_t u) { return adj_[u]; }
};

}  // namespace mt
