#ifndef ECHOTRACE_FRAGMENTS_HPP
#define ECHOTRACE_FRAGMENTS_HPP

#include "echotrace/document.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace echotrace
{

/// Matched-fragment retrieval between two parsed sources.
///
/// Given two documents, this finds the k-gram windows whose fingerprint hash is
/// shared and returns their original source spans, so reports and the web UI
/// can highlight *where* the two files coincide.
namespace fragments
{

/// A single shared k-gram occurrence, with source spans in both documents.
struct MatchedFragment
{
    std::uint64_t hash = 0;
    std::size_t a_token_pos = 0;
    std::size_t b_token_pos = 0;
    std::size_t a_offset = 0;
    std::size_t a_length = 0;
    std::size_t b_offset = 0;
    std::size_t b_length = 0;
    std::string a_text;
    std::string b_text;
};

/// Retrieve shared fragments between [a] and [b] using k-gram window size [k].
///
/// For every hash present in both fingerprints, up to [max_per_hash] position
/// pairs are emitted (capped to avoid quadratic blow-ups when a hash recurs).
/// The total number of fragments is capped at [max_total]. The returned vector
/// is ordered by first appearance in [a].
///
/// Complexity: O(shared_hashes * max_per_hash).
std::vector<MatchedFragment> matched_fragments(const ParsedSource& a,
                                               const ParsedSource& b,
                                               std::size_t k,
                                               std::size_t max_per_hash = 3,
                                               std::size_t max_total = 30);

} // namespace fragments

} // namespace echotrace

#endif // ECHOTRACE_FRAGMENTS_HPP
