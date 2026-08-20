#include "demos.h"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

namespace {

using stlpp_examples::Demo;

const Demo kDemos[] = {
    {"orbit", "Orbit forensics (ρ-structure)",
     "Floyd collision + connection_point on a modular map — stem & cycle lengths.",
     &stlpp_examples::demo_orbit_rho},
    {"fibpower", "Fibonacci via matrix power",
     "O(log n) F_n using mt::power on an associative 2×2 matrix monoid.",
     &stlpp_examples::demo_matrix_fibonacci},
    {"hotspots", "Pattern hotspots pipeline",
     "KMP matches → prefix scan → window scores → parallel sort (psort).",
     &stlpp_examples::demo_pattern_hotspots},
};

void print_menu()
{
    std::cout << "\n";
    std::cout << "  STL++ example runner\n";
    std::cout << "  --------------------\n";
    std::cout << "  Pick a demo that exercises real algorithms (not toy API calls).\n\n";
    for (std::size_t i = 0; i < std::size(kDemos); ++i) {
        std::cout << "  [" << (i + 1) << "]  " << kDemos[i].title << "\n";
        std::cout << "       key=" << kDemos[i].key << "\n";
        std::cout << "       " << kDemos[i].blurb << "\n\n";
    }
    std::cout << "  [q]  quit\n\n";
    std::cout << "Choice (number or key): " << std::flush;
}

const Demo* find_demo(std::string_view choice)
{
    if (choice.empty()) {
        return nullptr;
    }
    if (choice.size() == 1 && std::isdigit(static_cast<unsigned char>(choice[0]))) {
        const std::size_t idx = static_cast<std::size_t>(choice[0] - '0');
        if (idx >= 1 && idx <= std::size(kDemos)) {
            return &kDemos[idx - 1];
        }
    }
    for (const auto& d : kDemos) {
        if (d.key == choice) {
            return &d;
        }
    }
    return nullptr;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc > 1) {
        const auto* d = find_demo(argv[1]);
        if (d == nullptr) {
            std::cerr << "Unknown demo '" << argv[1] << "'. Try: ";
            for (std::size_t i = 0; i < std::size(kDemos); ++i) {
                std::cerr << kDemos[i].key << (i + 1 < std::size(kDemos) ? ", " : "\n");
            }
            return 1;
        }
        d->run();
        return 0;
    }

    for (;;) {
        print_menu();
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (line == "q" || line == "Q" || line == "quit") {
            break;
        }
        const auto* d = find_demo(line);
        if (d == nullptr) {
            std::cout << "Unknown choice '" << line << "'.\n";
            continue;
        }
        d->run();
        std::cout << "\n(press Enter to return to menu)\n";
        std::string pause;
        std::getline(std::cin, pause);
    }
    return 0;
}
