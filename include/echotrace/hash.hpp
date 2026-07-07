#ifndef ECHOTRACE_HASH_HPP
#define ECHOTRACE_HASH_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace echotrace
{

/// K-gram rolling hashing.
///
/// Each token symbol is mapped to a numeric value (see ``symbol``) and a
/// polynomial rolling hash of window [k] is computed over the token stream.
/// Rolling evaluation keeps the per-file cost at O(n) regardless of k.
namespace hash
{

/// Map a single token symbol to a numeric value in [1, mod).
/// Exposed for unit testing of the rolling recurrence.
std::uint64_t symbol(std::string_view token);

/// Compute the polynomial rolling hash of every length-[k] window over
/// [tokens] using a rolling recurrence.
///
/// Complexity: O(tokens.size()). Throws std::invalid_argument if k == 0.
/// Returns an empty vector when tokens.size() < k.
std::vector<std::uint64_t> rolling_token_hashes(const std::vector<std::string>& tokens,
                                               std::size_t k);

/// Internal modulus and base, exposed for diagnostics and tests.
inline constexpr std::uint64_t kBase = 256;
inline constexpr std::uint64_t kMod  = 1'000'000'007;

} // namespace hash

} // namespace echotrace

#endif // ECHOTRACE_HASH_HPP
