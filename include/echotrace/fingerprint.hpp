#ifndef ECHOTRACE_FINGERPRINT_HPP
#define ECHOTRACE_FINGERPRINT_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace echotrace
{

/// Winnowing fingerprint selection.
///
/// Given the rolling-hash sequence produced by ``hash::rolling_token_hashes``,
/// winnowing selects a small, representative subset that guarantees any shared
/// substring of length >= window+ k-1 tokens contributes at least one selected
/// fingerprint. This keeps the fingerprint set compact while preserving the
/// ability to detect localised overlap.
namespace fingerprint
{

/// A selected fingerprint: the token-index [pos] of the k-gram whose hash was
/// chosen, together with the hash value [hash].
struct FingerprintPick
{
    std::size_t pos = 0;
    std::uint64_t hash = 0;
};

/// Select winnowing fingerprints from [hashes] using a sliding window of
/// size [window], retaining the token position of every pick.
///
/// Complexity: O(hashes.size()) amortised (monotonic deque).
/// Throws std::invalid_argument if window == 0.
std::vector<FingerprintPick> winnow_picks(const std::vector<std::uint64_t>& hashes,
                                          std::size_t window);

/// Select winnowing fingerprints from [hashes] as a set of hash values.
///
/// Equivalent to the unique hash values of winnow_picks(), preserved verbatim
/// from the original implementation so existing similarity results are stable.
///
/// Complexity: O(hashes.size()) amortised. Throws std::invalid_argument if
/// window == 0.
std::unordered_set<std::uint64_t> winnow(const std::vector<std::uint64_t>& hashes,
                                         std::size_t window);

/// Convenience: tokenise [src], hash with window [k], winnow with [window].
std::unordered_set<std::uint64_t> from_source(std::string_view src,
                                              std::size_t k,
                                              std::size_t window);

} // namespace fingerprint

} // namespace echotrace

#endif // ECHOTRACE_FINGERPRINT_HPP
