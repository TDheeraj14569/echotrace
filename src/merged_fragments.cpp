#include "echotrace/merged_fragments.hpp"

#include <algorithm>

namespace echotrace::fragments {

std::vector<MergedSpan> merge_fragments(
    std::vector<MatchedFragment> frags,
    std::size_t merge_gap) {
    
    if (frags.empty()) {
        return {};
    }

    std::sort(frags.begin(), frags.end(), [](const MatchedFragment& lhs, const MatchedFragment& rhs) {
        if (lhs.a_offset != rhs.a_offset) {
            return lhs.a_offset < rhs.a_offset;
        }
        return lhs.b_offset < rhs.b_offset;
    });

    std::vector<MergedSpan> merged;
    
    MergedSpan current;
    current.a_offset = frags[0].a_offset;
    current.a_length = frags[0].a_length;
    current.b_offset = frags[0].b_offset;
    current.b_length = frags[0].b_length;
    current.fragment_count = 1;

    for (std::size_t i = 1; i < frags.size(); ++i) {
        const auto& f = frags[i];
        
        if (f.a_offset <= current.a_offset + current.a_length + merge_gap) {
            std::size_t b_start = std::min(current.b_offset, f.b_offset);
            std::size_t b_end_current = current.b_offset + current.b_length;
            std::size_t b_end_f = f.b_offset + f.b_length;
            std::size_t b_end = std::max(b_end_current, b_end_f);
            
            if (b_end - b_start <= current.b_length + f.b_length + merge_gap) {
                current.a_length = std::max(current.a_offset + current.a_length, f.a_offset + f.a_length) - current.a_offset;
                current.b_offset = b_start;
                current.b_length = b_end - b_start;
                current.fragment_count++;
                continue;
            }
        }
        
        merged.push_back(current);
        current.a_offset = f.a_offset;
        current.a_length = f.a_length;
        current.b_offset = f.b_offset;
        current.b_length = f.b_length;
        current.fragment_count = 1;
    }
    merged.push_back(current);

    return merged;
}

std::vector<MergedSpan> filter_small(
    const std::vector<MergedSpan>& spans,
    std::size_t min_length) {
    
    std::vector<MergedSpan> filtered;
    filtered.reserve(spans.size());
    for (const auto& s : spans) {
        if (s.a_length >= min_length || s.b_length >= min_length) {
            filtered.push_back(s);
        }
    }
    return filtered;
}

} // namespace echotrace::fragments
