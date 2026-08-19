#pragma once

#include "split.h"

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace mt {

/// Split on `\\n` (keeps empty lines so `unlines(lines(s))` round-trips
/// when the input does not end with a newline).
///
/// Example:
///   mt::lines("a\\nb\\n");  // {"a", "b", ""}
[[nodiscard]] inline std::vector<std::string> lines(std::string_view text)
{
    return split_copy(text, "\n");
}

/// Join lines with `\\n`.
///
/// Example:
///   mt::unlines(std::vector<std::string>{"a", "b"});  // "a\\nb"
[[nodiscard]] inline std::string unlines(const std::vector<std::string>& rows)
{
    return join(rows, "\n");
}

/// Split on ASCII whitespace; consecutive whitespace collapses (no empty tokens).
///
/// Example:
///   mt::words("  one  two three ");  // {"one", "two", "three"}
[[nodiscard]] inline std::vector<std::string> words(std::string_view text)
{
    std::vector<std::string> out;
    std::string token;
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!token.empty()) {
                out.push_back(std::move(token));
                token.clear();
            }
        } else {
            token.push_back(c);
        }
    }
    if (!token.empty()) {
        out.push_back(std::move(token));
    }
    return out;
}

/// Join words with a single space.
///
/// Example:
///   mt::unwords(std::vector<std::string>{"one", "two"});  // "one two"
[[nodiscard]] inline std::string unwords(const std::vector<std::string>& tokens)
{
    return join(tokens, " ");
}

}  // namespace mt
