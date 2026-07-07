#ifndef ECHOTRACE_DOCUMENT_HPP
#define ECHOTRACE_DOCUMENT_HPP

#include "echotrace/lexer.hpp"
#include "echotrace/normalization.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace echotrace
{

/// Immutable, fully-parsed representation of one source file.
///
/// A ParsedSource is built once and then reused by every pairwise comparison
/// that involves the file, satisfying the Phase 1 requirement that each file is
/// loaded, tokenised, normalised, hashed and winnowed exactly once.
struct ParsedSource
{
    /// Display path (relative to the scanned root where possible).
    std::string path;

    /// Original source text, retained for matched-fragment highlighting.
    std::string source;

    /// Normalised token stream as plain symbols.
    std::vector<std::string> tokens;

    /// Same tokens with their original source character spans.
    std::vector<lex::Token> token_spans;

    /// Rolling hashes of every k-gram.
    std::vector<std::uint64_t> hashes;

    /// Winnowing fingerprint set used by the similarity engine.
    std::unordered_set<std::uint64_t> fingerprint;

    /// Maps each fingerprint hash value to the token start indices at which its
    /// k-gram occurs. Drives matched-fragment retrieval. Only hashes that are
    /// members of [fingerprint] appear here.
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> fingerprint_index;

    /// Number of k-grams produced (== hashes.size()).
    std::size_t kgrams = 0;

    /// True when the source yielded at least one token.
    bool had_tokens = false;

    /// Statistics from the normalisation stage.
    normalization::NormalizationStats norm_stats{};
};

/// Lightweight per-document profile, kept for backward compatibility with the
/// original two-file report shape (tokens / k-grams / fingerprint).
struct DocumentProfile
{
    std::size_t tokens = 0;
    std::size_t kgrams = 0;
    std::unordered_set<std::uint64_t> fingerprint;
};

/// Build a DocumentProfile view of a parsed source.
DocumentProfile to_profile(const ParsedSource& src);

/// Parse [src] once into an immutable ParsedSource.
///
/// Pipeline: tokenize -> normalise -> rolling-hash(k) -> winnow(window).
/// Complexity: O(src.size()) amortised.
ParsedSource parse_source(std::string_view src,
                          std::size_t k,
                          std::size_t window,
                          const normalization::NormalizationOptions& opts);

} // namespace echotrace

#endif // ECHOTRACE_DOCUMENT_HPP
