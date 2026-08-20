#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace mt {

/// Knuth–Morris–Pratt: every starting offset where `pattern` occurs in `text`.
/// Empty pattern yields an empty result. Overlapping matches are reported
/// (e.g. "aaa" in "aaaa" -> {0, 1}).
///
/// Flow: build the prefix table `pi` in O(m); scan the text in O(n). On a
/// mismatch jump `j = pi[j-1]` instead of restarting. On a full match record
/// `i+1-m` and continue with `j = pi[m-1]`.
///
/// Example:
///   mt::matches("ababcababa", "aba");  // {0, 5, 7}
[[nodiscard]] inline std::vector<std::size_t> matches(std::string_view text,
                                                      std::string_view pattern)
{
    if (pattern.empty() || pattern.size() > text.size()) {
        return {};
    }

    std::vector<std::size_t> pi(pattern.size(), 0);
    for (std::size_t i = 1, j = 0; i < pattern.size(); ++i) {
        while (j > 0 && pattern[i] != pattern[j]) {
            j = pi[j - 1];
        }
        if (pattern[i] == pattern[j]) {
            ++j;
        }
        pi[i] = j;
    }

    std::vector<std::size_t> hits;
    for (std::size_t i = 0, j = 0; i < text.size(); ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            j = pi[j - 1];
        }
        if (text[i] == pattern[j]) {
            ++j;
        }
        if (j == pattern.size()) {
            hits.push_back(i + 1 - j);
            j = pi[j - 1];
        }
    }
    return hits;
}

}  // namespace mt
