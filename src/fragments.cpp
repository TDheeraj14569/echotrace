#include "echotrace/fragments.hpp"

#include <algorithm>
#include <utility>

namespace echotrace
{

namespace fragments
{

namespace
{

// Source span [offset, length) covering tokens [first, last] (inclusive) of a
// parsed source. Returns an empty span if the range is out of bounds.
struct Span
{
    std::size_t offset = 0;
    std::size_t length = 0;
    bool valid = false;
};

Span token_range_span(const ParsedSource& s, std::size_t first, std::size_t last)
{
    if (s.token_spans.empty() || first > last || last >= s.token_spans.size())
    {
        return {};
    }
    const auto& begin_tok = s.token_spans[first];
    const auto& end_tok = s.token_spans[last];
    if (begin_tok.offset > s.source.size() ||
        end_tok.offset + end_tok.length > s.source.size())
    {
        return {};
    }
    Span sp;
    sp.offset = begin_tok.offset;
    sp.length = (end_tok.offset + end_tok.length) - begin_tok.offset;
    sp.valid = true;
    return sp;
}

} // namespace

std::vector<MatchedFragment> matched_fragments(const ParsedSource& a,
                                               const ParsedSource& b,
                                               std::size_t k,
                                               std::size_t max_per_hash,
                                               std::size_t max_total)
{
    std::vector<MatchedFragment> out;
    if (k == 0 || max_total == 0)
    {
        return out;
    }

    // Iterate the smaller fingerprint set for the intersection.
    const auto& small = (a.fingerprint.size() < b.fingerprint.size()) ? a.fingerprint : b.fingerprint;
    const auto& large = (a.fingerprint.size() < b.fingerprint.size()) ? b.fingerprint : a.fingerprint;

    for (const auto h : small)
    {
        if (out.size() >= max_total)
        {
            break;
        }
        if (large.find(h) == large.end())
        {
            continue;
        }

        const auto ia = a.fingerprint_index.find(h);
        const auto ib = b.fingerprint_index.find(h);
        if (ia == a.fingerprint_index.end() || ib == b.fingerprint_index.end())
        {
            continue;
        }

        std::size_t emitted = 0;
        for (const auto pa : ia->second)
        {
            if (emitted >= max_per_hash || out.size() >= max_total)
            {
                break;
            }
            for (const auto pb : ib->second)
            {
                if (emitted >= max_per_hash || out.size() >= max_total)
                {
                    break;
                }

                // A valid k-gram window needs tokens [pos, pos+k-1].
                if (pa + k > a.token_spans.size() || pb + k > b.token_spans.size())
                {
                    continue;
                }

                const auto sa = token_range_span(a, pa, pa + k - 1);
                const auto sb = token_range_span(b, pb, pb + k - 1);
                if (!sa.valid || !sb.valid)
                {
                    continue;
                }

                MatchedFragment f;
                f.hash = h;
                f.a_token_pos = pa;
                f.b_token_pos = pb;
                f.a_offset = sa.offset;
                f.a_length = sa.length;
                f.b_offset = sb.offset;
                f.b_length = sb.length;
                f.a_text = a.source.substr(sa.offset, sa.length);
                f.b_text = b.source.substr(sb.offset, sb.length);
                out.push_back(std::move(f));
                ++emitted;
            }
        }
    }

    return out;
}

} // namespace fragments

} // namespace echotrace
