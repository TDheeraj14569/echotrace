#ifndef ECHOTRACE_NORMALIZATION_HPP
#define ECHOTRACE_NORMALIZATION_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace echotrace
{

/// Token-stream normalisation passes that run *after* lexical normalisation.
///
/// The lexer already performs the bulk of normalisation: identifiers collapse
/// to ``"V"``, types to ``"T"``, numeric literals to ``"N"``, string/char
/// literals to ``"L"``, and whitespace/comments are discarded. The passes in
/// this namespace go further, but ONLY where semantic equivalence is
/// guaranteed, so that equivalent programs always produce equivalent
/// fingerprints.
///
/// Excluded by design (correctness NOT guaranteed):
///   * ``a += b``  <--> ``a = a + b``   (differs for overloaded operators)
///   * ``++x``     <--> ``x += 1``      (differs for overloaded/volatile)
///   * ``while(N)``  infinite-loop fold  (``N`` may be ``0``)
///   * removal of a *single* paren pair  (changes precedence)
namespace normalization
{

/// Toggles for individual normalisation passes. Every pass defaults to a safe,
/// semantics-preserving transformation.
struct NormalizationOptions
{
    /// Collapse immediately-nested parentheses: ``((E))`` -> ``(E)``.
    /// Provably safe: grouping an already-grouped expression changes nothing.
    bool collapse_redundant_parens = true;
};

/// Statistics accumulated during normalisation. Useful for reports.
struct NormalizationStats
{
    std::size_t parens_collapsed = 0;
};

/// The default, safe option set used by the analysis engine.
inline const NormalizationOptions& default_options()
{
    static constexpr NormalizationOptions defaults{};
    return defaults;
}

/// Compute a survivor mask over [tokens]: 1 == keep, 0 == drop.
///
/// Every enabled pass contributes to the same mask. Returning a mask (rather
/// than a new token vector) lets callers keep parallel token / source-span
/// vectors perfectly aligned, with no second guessing of what was removed.
///
/// Complexity: O(tokens.size()) amortised per enabled pass.
std::vector<char> keep_mask(const std::vector<std::string>& tokens,
                            const NormalizationOptions& options,
                            NormalizationStats& stats);

/// Convenience: apply the survivor mask to produce a fresh token vector.
std::vector<std::string> normalize_tokens(std::vector<std::string> tokens,
                                          const NormalizationOptions& options,
                                          NormalizationStats& stats);

} // namespace normalization

} // namespace echotrace

#endif // ECHOTRACE_NORMALIZATION_HPP
