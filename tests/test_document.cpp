#include "test_framework.hpp"
#include "echotrace/document.hpp"

TEST(DocumentProducesTokens) {
    echotrace::normalization::NormalizationOptions opts{};
    auto doc = echotrace::parse_source("int main() { return 0; }", 5, 4, opts);
    ASSERT_TRUE(doc.had_tokens);
    ASSERT_FALSE(doc.tokens.empty());
}

TEST(DocumentProducesFingerprints) {
    echotrace::normalization::NormalizationOptions opts{};
    auto doc = echotrace::parse_source("int main() { return 0; }", 5, 4, opts);
    ASSERT_FALSE(doc.fingerprint.empty());
}

TEST(DocumentFingerprintIndex) {
    echotrace::normalization::NormalizationOptions opts{};
    auto doc = echotrace::parse_source("int main() { return 0; }", 5, 4, opts);
    ASSERT_FALSE(doc.fingerprint_index.empty());
    // Every key in fingerprint_index should be in fingerprint
    for (const auto& [hash, positions] : doc.fingerprint_index) {
        ASSERT_TRUE(doc.fingerprint.count(hash) > 0);
    }
}

TEST(DocumentEmptySource) {
    echotrace::normalization::NormalizationOptions opts{};
    auto doc = echotrace::parse_source("", 5, 4, opts);
    ASSERT_FALSE(doc.had_tokens);
    ASSERT_TRUE(doc.tokens.empty());
    ASSERT_TRUE(doc.fingerprint.empty());
}

TEST(DocumentToProfile) {
    echotrace::normalization::NormalizationOptions opts{};
    auto doc = echotrace::parse_source("int main() { return 0; }", 5, 4, opts);
    auto prof = echotrace::to_profile(doc);
    ASSERT_EQ(prof.tokens, doc.tokens.size());
    ASSERT_EQ(prof.kgrams, doc.kgrams);
    ASSERT_EQ(prof.fingerprint.size(), doc.fingerprint.size());
}

TEST(DocumentSourcePreserved) {
    echotrace::normalization::NormalizationOptions opts{};
    std::string src = "int x = 42;";
    auto doc = echotrace::parse_source(src, 5, 4, opts);
    ASSERT_EQ(doc.source, src);
}
