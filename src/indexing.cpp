#include "echotrace/indexing.hpp"

#include <algorithm>
#include <map>

namespace echotrace::indexing {

InvertedIndex build_index(const std::vector<ParsedSource>& sources) {
    InvertedIndex idx;
    idx.total_files = sources.size();
    
    for (std::size_t i = 0; i < sources.size(); ++i) {
        for (const auto& hash : sources[i].fingerprint) {
            idx.index[hash].push_back(i);
        }
    }
    
    return idx;
}

std::vector<std::pair<std::size_t, std::size_t>> candidate_pairs(
    const InvertedIndex& index,
    std::size_t min_shared) {
    
    // pair -> count of shared hashes
    std::map<std::pair<std::size_t, std::size_t>, std::size_t> pair_counts;
    
    for (const auto& [hash, doc_indices] : index.index) {
        if (doc_indices.size() < 2) continue;
        
        for (std::size_t i = 0; i < doc_indices.size() - 1; ++i) {
            for (std::size_t j = i + 1; j < doc_indices.size(); ++j) {
                auto u = doc_indices[i];
                auto v = doc_indices[j];
                if (u > v) std::swap(u, v);
                if (u != v) {
                    pair_counts[{u, v}]++;
                }
            }
        }
    }
    
    std::vector<std::pair<std::size_t, std::size_t>> result;
    for (const auto& [p, count] : pair_counts) {
        if (count >= min_shared) {
            result.push_back(p);
        }
    }
    
    return result;
}

} // namespace echotrace::indexing
