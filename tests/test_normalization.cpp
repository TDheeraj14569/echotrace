#include "test_framework.hpp"
#include "echotrace/normalization.hpp"
#include <vector>
#include <string>

TEST(NormalizationCollapseRedundantParens) {
    // ((x)) -> (x): inner parens are redundant
    std::vector<std::string> tokens = {"(", "(", "V", ")", ")"};
    echotrace::normalization::NormalizationStats stats{};
    echotrace::normalization::NormalizationOptions opts{};
    auto mask = echotrace::normalization::keep_mask(tokens, opts, stats);
    ASSERT_EQ(mask.size(), static_cast<size_t>(5));
    // The inner ( at [1] preceded by ( at [0] is redundant.
    // The outer pair stays, inner pair drops.
    ASSERT_GT(stats.parens_collapsed, static_cast<size_t>(0));
}

TEST(NormalizationSingleParensKept) {
    std::vector<std::string> tokens = {"(", "V", ")"};
    echotrace::normalization::NormalizationStats stats{};
    echotrace::normalization::NormalizationOptions opts{};
    auto mask = echotrace::normalization::keep_mask(tokens, opts, stats);
    ASSERT_EQ(mask.size(), static_cast<size_t>(3));
    ASSERT_EQ(mask[0], static_cast<char>(1));
    ASSERT_EQ(mask[1], static_cast<char>(1));
    ASSERT_EQ(mask[2], static_cast<char>(1));
    ASSERT_EQ(stats.parens_collapsed, static_cast<size_t>(0));
}

TEST(NormalizationEmpty) {
    std::vector<std::string> tokens;
    echotrace::normalization::NormalizationStats stats{};
    echotrace::normalization::NormalizationOptions opts{};
    auto mask = echotrace::normalization::keep_mask(tokens, opts, stats);
    ASSERT_EQ(mask.size(), static_cast<size_t>(0));
}

TEST(NormalizationMismatchedParens) {
    // Mismatched parens should be left alone
    std::vector<std::string> tokens = {"(", "(", "V", ")"};
    echotrace::normalization::NormalizationStats stats{};
    echotrace::normalization::NormalizationOptions opts{};
    auto mask = echotrace::normalization::keep_mask(tokens, opts, stats);
    ASSERT_EQ(mask.size(), static_cast<size_t>(4));
}

TEST(NormalizationDisabled) {
    std::vector<std::string> tokens = {"(", "(", "V", ")", ")"};
    echotrace::normalization::NormalizationStats stats{};
    echotrace::normalization::NormalizationOptions opts{};
    opts.collapse_redundant_parens = false;
    auto mask = echotrace::normalization::keep_mask(tokens, opts, stats);
    // All kept when disabled
    for (auto c : mask) {
        ASSERT_EQ(c, static_cast<char>(1));
    }
}

TEST(NormalizationNormalizeTokens) {
    std::vector<std::string> tokens = {"(", "(", "V", ")", ")"};
    echotrace::normalization::NormalizationStats stats{};
    echotrace::normalization::NormalizationOptions opts{};
    auto result = echotrace::normalization::normalize_tokens(tokens, opts, stats);
    // Should have fewer tokens (redundant parens removed)
    ASSERT_LT(result.size(), tokens.size());
}
