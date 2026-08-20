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

}  // namespace stlpp_examples
