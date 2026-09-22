#pragma once

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace mt {

struct point_2d {
    double x = 0;
    double y = 0;
};

struct rect_2d {
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;

    [[nodiscard]] bool contains(point_2d p) const
    {
        return p.x >= x0 && p.x < x1 && p.y >= y0 && p.y < y1;
    }

    [[nodiscard]] bool intersects(rect_2d o) const
    {
        return x0 < o.x1 && x1 > o.x0 && y0 < o.y1 && y1 > o.y0;
    }
};

/// Point quadtree. Nodes split at `capacity` points into four quadrants.
///
/// Example:
///   mt::quadtree q({0, 0, 100, 100});
///   q.insert({10, 10});
///   auto hits = q.query({0, 0, 20, 20});
class quadtree {
    static constexpr std::size_t capacity = 4;

    struct node {
        rect_2d box;
        std::vector<point_2d> points;
        std::unique_ptr<node> nw;
        std::unique_ptr<node> ne;
        std::unique_ptr<node> sw;
        std::unique_ptr<node> se;

        explicit node(rect_2d b) : box(b) {}

        [[nodiscard]] bool divided() const { return static_cast<bool>(nw); }

        bool divide()
        {
            const double mx = std::midpoint(box.x0, box.x1);
            const double my = std::midpoint(box.y0, box.y1);
            if (!(box.x0 < mx && mx < box.x1 && box.y0 < my && my < box.y1)) {
                return false;
            }
            // Publish all four children together, including on allocation failure.
            auto new_nw = std::make_unique<node>(rect_2d{box.x0, my, mx, box.y1});
            auto new_ne = std::make_unique<node>(rect_2d{mx, my, box.x1, box.y1});
            auto new_sw = std::make_unique<node>(rect_2d{box.x0, box.y0, mx, my});
            auto new_se = std::make_unique<node>(rect_2d{mx, box.y0, box.x1, my});
            nw = std::move(new_nw);
            ne = std::move(new_ne);
            sw = std::move(new_sw);
            se = std::move(new_se);
            return true;
        }
    };

    std::unique_ptr<node> root_;

    static bool insert_into(node& n, point_2d p)
    {
        if (!n.box.contains(p)) {
            return false;
        }
        auto into_child = [&](point_2d q) {
            return insert_into(*n.nw, q) || insert_into(*n.ne, q) || insert_into(*n.sw, q) ||
                   insert_into(*n.se, q);
        };

        if (!n.divided()) {
            if (n.points.size() < capacity) {
                n.points.push_back(p);
                return true;
            }
            if (!n.divide()) {
                n.points.push_back(p);
                return true;
            }
            // Keep existing points here. Queries visit both this node and its
            // children, and failed child insertion cannot duplicate old points.
        }
        if (into_child(p)) {
            return true;
        }
        n.points.push_back(p);
        return true;
    }

    static void query_into(const node& n, rect_2d range, std::vector<point_2d>& out)
    {
        if (!n.box.intersects(range)) {
            return;
        }
        for (point_2d p : n.points) {
            if (range.contains(p)) {
                out.push_back(p);
            }
        }
        if (n.divided()) {
            query_into(*n.nw, range, out);
            query_into(*n.ne, range, out);
            query_into(*n.sw, range, out);
            query_into(*n.se, range, out);
        }
    }

public:
    explicit quadtree(rect_2d bounds) : root_(std::make_unique<node>(bounds)) {}

    bool insert(point_2d p) { return insert_into(*root_, p); }

    [[nodiscard]] std::vector<point_2d> query(rect_2d range) const
    {
        std::vector<point_2d> out;
        query_into(*root_, range, out);
        return out;
    }
};

}  // namespace mt
