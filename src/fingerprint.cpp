#include "echotrace/fingerprint.hpp"

#include "echotrace/hash.hpp"
#include "echotrace/lexer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <stdexcept>
#include <utility>

namespace echotrace
{

namespace fingerprint
{

std::vector<FingerprintPick> winnow_picks(const std::vector<std::uint64_t>& hashes,
                                          std::size_t window)
{
    if (window == 0)
    {
        throw std::invalid_argument("winnowing window must be positive");
    }

    std::vector<FingerprintPick> picks;
    if (hashes.empty())
    {
        return picks;
    }

    std::deque<std::pair<std::size_t, std::uint64_t>> mins;
    picks.reserve(hashes.size() / window + 1);

    std::size_t last_pick = hashes.size();

    for (std::size_t i = 0; i < hashes.size(); ++i)
    {
        while (!mins.empty() && mins.front().first + window <= i)
        {
            mins.pop_front();
        }

        while (!mins.empty() && mins.back().second >= hashes[i])
        {
            mins.pop_back();
        }
        mins.emplace_back(i, hashes[i]);

        if (i + 1 >= window && mins.front().first != last_pick)
        {
            picks.push_back({mins.front().first, mins.front().second});
            last_pick = mins.front().first;
        }
    }

    if (hashes.size() < window)
    {
        picks.push_back({mins.front().first, mins.front().second});
    }

    return picks;
}

std::unordered_set<std::uint64_t> winnow(const std::vector<std::uint64_t>& hashes,
                                         std::size_t window)
{
    const auto picks = winnow_picks(hashes, window);
    std::unordered_set<std::uint64_t> fp;
    fp.reserve(picks.size());
    for (const auto& p : picks)
    {
        fp.insert(p.hash);
    }
    return fp;
}

std::unordered_set<std::uint64_t> from_source(std::string_view src,
                                              std::size_t k,
                                              std::size_t window)
{
    auto tokens = lex::tokenize(src);
    auto hashes = hash::rolling_token_hashes(tokens, k);
    return winnow(hashes, window);
}

} // namespace fingerprint

} // namespace echotrace
