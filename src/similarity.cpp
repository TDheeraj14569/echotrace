#include "echotrace/similarity.hpp"

#include "echotrace/fingerprint.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_set>

namespace echotrace
{

namespace sim
{

JaccardDetail jaccard_detail(const std::unordered_set<std::uint64_t>& a,
                             const std::unordered_set<std::uint64_t>& b)
{
    if (a.empty() && b.empty())
    {
        return {100.0, 0, 0};
    }

    const auto& small = (a.size() < b.size()) ? a : b;
    const auto& large = (a.size() < b.size()) ? b : a;

    std::size_t intersection = 0;
    for (const auto h : small)
    {
        intersection += large.find(h) != large.end() ? 1u : 0u;
    }

    const auto uni = a.size() + b.size() - intersection;
    const double pct = uni == 0 ? 100.0
                                : (100.0 * static_cast<double>(intersection)) / static_cast<double>(uni);
    return {pct, intersection, uni};
}

double jaccard(const std::unordered_set<std::uint64_t>& a,
               const std::unordered_set<std::uint64_t>& b)
{
    return jaccard_detail(a, b).percent;
}

double compare(std::string_view lhs,
               std::string_view rhs,
               std::size_t k,
               std::size_t window)
{
    const auto a = fingerprint::from_source(lhs, k, window);
    const auto b = fingerprint::from_source(rhs, k, window);
    return jaccard(a, b);
}

} // namespace sim

} // namespace echotrace
