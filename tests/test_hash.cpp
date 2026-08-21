#include "test_framework.hpp"
#include "echotrace/hash.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>

TEST(HashSymbolConsistent) {
    auto h1 = echotrace::hash::symbol("test_token");
    auto h2 = echotrace::hash::symbol("test_token");
    ASSERT_EQ(h1, h2);
}

TEST(HashSymbolDifferent) {
    auto h1 = echotrace::hash::symbol("V");
    auto h2 = echotrace::hash::symbol("T");
    ASSERT_NE(h1, h2);
}

TEST(HashSymbolNonZero) {
    // symbol() maps to [1, mod), never 0
    auto h = echotrace::hash::symbol("V");
    ASSERT_GT(h, static_cast<std::uint64_t>(0));
}

TEST(HashRollingK0Throws) {
    std::vector<std::string> tokens = {"a", "b"};
    ASSERT_THROW(echotrace::hash::rolling_token_hashes(tokens, 0), std::invalid_argument);
}

TEST(HashRollingTooFewTokens) {
    std::vector<std::string> tokens = {"a", "b"};
    auto hashes = echotrace::hash::rolling_token_hashes(tokens, 3);
    ASSERT_EQ(hashes.size(), static_cast<size_t>(0));
}

TEST(HashRollingCorrectCount) {
    std::vector<std::string> tokens = {"a", "b", "c", "d", "e"};
    auto hashes = echotrace::hash::rolling_token_hashes(tokens, 3);
    // 5 tokens, k=3 -> 5-3+1 = 3 hashes
    ASSERT_EQ(hashes.size(), static_cast<size_t>(3));
}

TEST(HashRollingDeterministic) {
    std::vector<std::string> tokens = {"V", "=", "N", ";", "return"};
    auto h1 = echotrace::hash::rolling_token_hashes(tokens, 3);
    auto h2 = echotrace::hash::rolling_token_hashes(tokens, 3);
    ASSERT_EQ(h1.size(), h2.size());
    for (size_t i = 0; i < h1.size(); ++i) {
        ASSERT_EQ(h1[i], h2[i]);
    }
}

TEST(HashRollingShortInput) {
    std::vector<std::string> tokens = {"V"};
    auto hashes = echotrace::hash::rolling_token_hashes(tokens, 1);
    ASSERT_EQ(hashes.size(), static_cast<size_t>(1));
}

TEST(HashRollingEmpty) {
    std::vector<std::string> tokens;
    auto hashes = echotrace::hash::rolling_token_hashes(tokens, 3);
    ASSERT_EQ(hashes.size(), static_cast<size_t>(0));
}
