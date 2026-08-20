#pragma once

#include <string_view>

namespace stlpp_examples {

struct Demo {
    std::string_view key;
    std::string_view title;
    std::string_view blurb;
    void (*run)();
};

void demo_orbit_rho();
void demo_matrix_fibonacci();
void demo_pattern_hotspots();
void demo_expr_btree();
void demo_rope_zipper();
void demo_parallel_race();
void demo_cards_gather();
void demo_components();
void demo_lazy_pipeline();
void demo_adaptive_sort();
void demo_quadtree_query();
void demo_dijkstra();
void demo_text_pipeline();
void demo_combinations();

}  // namespace stlpp_examples
