#include "echotrace/hash.hpp"

#include <stdexcept>
#include <string_view>

namespace echotrace
{

namespace hash
{

namespace
{

std::uint64_t pow_mod(std::uint64_t x, std::size_t exp)
{
    std::uint64_t out = 1;
    while (exp-- > 0)
    {
        out = (out * x) % kMod;
    }
    return out;
}

} // namespace

std::uint64_t symbol(std::string_view token)
{
    std::uint64_t h = 1469598103934665603ull;
    for (std::size_t i = 0; i < token.size(); ++i)
    {
        const unsigned char ch = static_cast<unsigned char>(token[i]);
        h ^= ch;
        h *= 1099511628211ull;
    }
    return (h % (kMod - 1)) + 1;
}

std::vector<std::uint64_t> rolling_token_hashes(const std::vector<std::string>& tokens,
                                               std::size_t k)
{
    if (k == 0)
    {
        throw std::invalid_argument("hash window size must be positive");
    }
    if (tokens.size() < k)
    {
        return {};
    }

    std::vector<std::uint64_t> syms;
    syms.reserve(tokens.size());
    for (const auto& token : tokens)
    {
        syms.push_back(symbol(token));
    }

    std::vector<std::uint64_t> hashes;
    hashes.reserve(tokens.size() - k + 1);

    std::uint64_t h = 0;
    for (std::size_t i = 0; i < k; ++i)
    {
        h = (h * kBase + syms[i]) % kMod;
    }
    hashes.push_back(h);

    // Token symbols, rather than bytes, keep aliases such as "V" and "T" from
    // being weighted by the spelling length of adjacent operators or keywords.
    const auto lead_factor = pow_mod(kBase, k - 1);
    for (std::size_t i = k; i < syms.size(); ++i)
    {
        const auto lead = (syms[i - k] * lead_factor) % kMod;
        h = (h + kMod - lead) % kMod;
        h = (h * kBase + syms[i]) % kMod;
        hashes.push_back(h);
    }

    return hashes;
}

} // namespace hash

} // namespace echotrace
