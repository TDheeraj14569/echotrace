#ifndef ECHOTRACE_SIMILARITY_HPP
#define ECHOTRACE_SIMILARITY_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_set>

namespace echotrace
{

/// Similarity scoring over fingerprint sets.
namespace sim
{

/// Detailed Jaccard result: percentage plus the raw intersection and universe
/// cardinalities. Returned together so callers (e.g. report writers) need only
/// one pass over the smaller set.
struct JaccardDetail
{
    double percent = 0.0;
    std::size_t intersection = 0;
    std::size_t universe = 0;
};

/// Compute Jaccard detail of two fingerprint sets. Two empty sets are defined
/// to be 100% similar.
///
/// Complexity: O(min(|a|, |b|)) lookups.
JaccardDetail jaccard_detail(const std::unordered_set<std::uint64_t>& a,
                             const std::unordered_set<std::uint64_t>& b);

/// Jaccard similarity of two fingerprint sets, expressed as a percentage
/// in the range [0, 100]. Two empty sets are defined to be 100% similar.
///
/// Convenience wrapper around jaccard_detail().
double jaccard(const std::unordered_set<std::uint64_t>& a,
               const std::unordered_set<std::uint64_t>& b);

/// Convenience: fingerprint two sources and return their Jaccard similarity.
double compare(std::string_view lhs,
               std::string_view rhs,
               std::size_t k,
               std::size_t window);

} // namespace sim

} // namespace echotrace

#endif // ECHOTRACE_SIMILARITY_HPP
