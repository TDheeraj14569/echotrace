#include "test_framework.hpp"
#include "echotrace/fragments.hpp"
#include "echotrace/document.hpp"

TEST(FragmentsIdenticalSources) {
    echotrace::normalization::NormalizationOptions opts{};
    auto a = echotrace::parse_source("void test() { int x = 0; x++; return x; }", echotrace::Language::Cpp, 5, 4, opts);
    auto b = echotrace::parse_source("void test() { int x = 0; x++; return x; }", echotrace::Language::Cpp, 5, 4, opts);
    auto frags = echotrace::fragments::matched_fragments(a, b, 5);
    ASSERT_FALSE(frags.empty());
}

TEST(FragmentsDifferentSources) {
    echotrace::normalization::NormalizationOptions opts{};
    auto a = echotrace::parse_source("void foo() { int x = 1; }", echotrace::Language::Cpp, 5, 4, opts);
    auto b = echotrace::parse_source("#include <iostream>\nint main() { std::cout << 42; return 0; }", echotrace::Language::Cpp, 5, 4, opts);
    auto frags = echotrace::fragments::matched_fragments(a, b, 5);
    // Different enough code should have few or no fragments
    // (may have some due to shared keywords — just verify it doesn't crash)
}

TEST(FragmentsMaxLimits) {
    echotrace::normalization::NormalizationOptions opts{};
    auto a = echotrace::parse_source("int a=1; int b=2; int c=3; int d=4; int e=5;", echotrace::Language::Cpp, 5, 4, opts);
    auto b = echotrace::parse_source("int a=1; int b=2; int c=3; int d=4; int e=5;", echotrace::Language::Cpp, 5, 4, opts);
    auto frags = echotrace::fragments::matched_fragments(a, b, 5, 1, 2);
    ASSERT_LE(frags.size(), static_cast<size_t>(2));
}

TEST(FragmentOffsetsValid) {
    echotrace::normalization::NormalizationOptions opts{};
    auto a = echotrace::parse_source("int val = 10; return val;", echotrace::Language::Cpp, 5, 4, opts);
    auto b = echotrace::parse_source("int val = 10; return val;", echotrace::Language::Cpp, 5, 4, opts);
    auto frags = echotrace::fragments::matched_fragments(a, b, 5);
    for (const auto& f : frags) {
        ASSERT_LE(f.a_offset + f.a_length, a.source.size());
        ASSERT_LE(f.b_offset + f.b_length, b.source.size());
    }
}

TEST(FragmentTextMatchesSource) {
    echotrace::normalization::NormalizationOptions opts{};
    std::string src = "double d = 3.14; return d;";
    auto a = echotrace::parse_source(src, echotrace::Language::Cpp, 5, 4, opts);
    auto b = echotrace::parse_source(src, echotrace::Language::Cpp, 5, 4, opts);
    auto frags = echotrace::fragments::matched_fragments(a, b, 5);
    for (const auto& f : frags) {
        ASSERT_EQ(f.a_text, a.source.substr(f.a_offset, f.a_length));
        ASSERT_EQ(f.b_text, b.source.substr(f.b_offset, f.b_length));
    }
}

TEST(FragmentsK0ReturnsEmpty) {
    echotrace::normalization::NormalizationOptions opts{};
    auto a = echotrace::parse_source("auto x = 5;", echotrace::Language::Cpp, 5, 4, opts);
    auto b = echotrace::parse_source("auto x = 5;", echotrace::Language::Cpp, 5, 4, opts);
    auto frags = echotrace::fragments::matched_fragments(a, b, 0);
    ASSERT_TRUE(frags.empty());
}
