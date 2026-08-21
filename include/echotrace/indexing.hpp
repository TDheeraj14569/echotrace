#ifndef ECHOTRACE_INDEXING_HPP
#define ECHOTRACE_INDEXING_HPP

#include "echotrace/document.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

namespace echotrace::indexing {

// Maps fingerprint hashes to file indices
struct InvertedIndex {
    // hash -> list of source indices containing that hash
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> index;
    std::size_t total_files = 0;
};

// Build an inverted index from parsed sources
InvertedIndex build_index(const std::vector<ParsedSource>& sources);

// Generate candidate pairs from the inverted index.
// A pair (i,j) is a candidate if the two files share at least min_shared fingerprints.
std::vector<std::pair<std::size_t, std::size_t>> candidate_pairs(
    const InvertedIndex& index,
    std::size_t min_shared = 1);

} // namespace echotrace::indexing

#endif // ECHOTRACE_INDEXING_HPP
