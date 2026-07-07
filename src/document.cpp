#include "echotrace/document.hpp"

#include "echotrace/fingerprint.hpp"
#include "echotrace/hash.hpp"
#include "echotrace/lexer.hpp"
#include "echotrace/normalization.hpp"

#include <utility>

namespace echotrace
{

DocumentProfile to_profile(const ParsedSource& src)
{
    return DocumentProfile{
        src.tokens.size(),
        src.kgrams,
        src.fingerprint,
    };
}

ParsedSource parse_source(std::string_view src,
                          std::size_t k,
                          std::size_t window,
                          const normalization::NormalizationOptions& opts)
{
    ParsedSource out;
    out.source = std::string(src);

    auto spans = lex::tokenize_spans(src);

    // Project to text, compute the survivor mask once, then apply it to both
    // the text and span vectors so they stay perfectly aligned.
    std::vector<std::string> texts;
    texts.reserve(spans.size());
    for (const auto& t : spans)
    {
        texts.push_back(t.text);
    }

    normalization::NormalizationStats stats{};
    const auto keep = normalization::keep_mask(texts, opts, stats);

    out.tokens.reserve(spans.size());
    out.token_spans.reserve(spans.size());
    for (std::size_t i = 0; i < spans.size(); ++i)
    {
        if (keep[i] != 0)
        {
            out.token_spans.push_back(std::move(spans[i]));
        }
    }
    for (const auto& t : out.token_spans)
    {
        out.tokens.push_back(t.text);
    }

    out.norm_stats = stats;
    out.had_tokens = !out.tokens.empty();

    out.hashes = hash::rolling_token_hashes(out.tokens, k);
    out.kgrams = out.hashes.size();
    out.fingerprint = fingerprint::winnow(out.hashes, window);

    // Build hash -> token positions index from winnow picks.
    const auto picks = fingerprint::winnow_picks(out.hashes, window);
    out.fingerprint_index.reserve(picks.size());
    for (const auto& p : picks)
    {
        out.fingerprint_index[p.hash].push_back(p.pos);
    }

    return out;
}

} // namespace echotrace
