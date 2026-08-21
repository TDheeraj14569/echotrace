#ifndef ECHOTRACE_MERGED_FRAGMENTS_HPP
#define ECHOTRACE_MERGED_FRAGMENTS_HPP

#include "echotrace/fragments.hpp"

#include <cstddef>
#include <vector>

namespace echotrace::fragments {

struct MergedSpan {
    std::size_t a_offset = 0;
    std::size_t a_length = 0;
    std::size_t b_offset = 0;
    std::size_t b_length = 0;
    std::size_t fragment_count = 0;  // how many k-grams were merged
};

// Merge adjacent/overlapping matched fragments into contiguous spans
std::vector<MergedSpan> merge_fragments(
    std::vector<MatchedFragment> frags,
    std::size_t merge_gap = 3);  // merge if fragments are within this many tokens

// Filter out small merged spans
std::vector<MergedSpan> filter_small(
    const std::vector<MergedSpan>& spans,
    std::size_t min_length = 10);  // minimum byte length

} // namespace echotrace::fragments

#endif // ECHOTRACE_MERGED_FRAGMENTS_HPP
