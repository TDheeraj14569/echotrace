#include "echotrace/normalization.hpp"

#include <utility>

namespace echotrace
{

namespace normalization
{

namespace
{

// Mark redundant immediately-nested parentheses for removal in [keep].
//
// A '(' whose immediately preceding token is also '(' is redundant, and its
// matching ')' is removed too. Single paren pairs are NEVER removed, so
// precedence is always preserved. Mismatched parentheses are left alone.
//
// Provenance of safety: in C++ a parenthesised expression has exactly the same
// value category and value as its operand; nesting parentheses therefore
// cannot change semantics.
void mark_redundant_parens(const std::vector<std::string>& tokens,
                           std::vector<char>& keep,
                           NormalizationStats& stats)
{
    // Each stack entry: (index of '(', was it redundant?).
    std::vector<std::pair<std::size_t, bool>> opens;
    opens.reserve(tokens.size() / 4 + 1);

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i] == "(")
        {
            // Redundant iff the immediately preceding token is also '('.
            const bool redundant = (i > 0 && tokens[i - 1] == "(");
            opens.emplace_back(i, redundant);
            if (redundant)
            {
                keep[i] = 0;
            }
        }
        else if (tokens[i] == ")")
        {
            if (!opens.empty())
            {
                const auto& top = opens.back();
                if (top.second)
                {
                    keep[i] = 0;
                    keep[top.first] = 0;
                    ++stats.parens_collapsed;
                }
                opens.pop_back();
            }
        }
    }
}

} // namespace

std::vector<char> keep_mask(const std::vector<std::string>& tokens,
                            const NormalizationOptions& options,
                            NormalizationStats& stats)
{
    std::vector<char> keep(tokens.size(), 1);
    if (options.collapse_redundant_parens)
    {
        mark_redundant_parens(tokens, keep, stats);
    }
    return keep;
}

std::vector<std::string> normalize_tokens(std::vector<std::string> tokens,
                                          const NormalizationOptions& options,
                                          NormalizationStats& stats)
{
    const auto keep = keep_mask(tokens, options, stats);

    std::size_t kept = 0;
    for (char c : keep)
    {
        kept += static_cast<std::size_t>(c);
    }

    std::vector<std::string> out;
    out.reserve(kept);
    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (keep[i] != 0)
        {
            out.push_back(std::move(tokens[i]));
        }
    }
    return out;
}

} // namespace normalization

} // namespace echotrace
