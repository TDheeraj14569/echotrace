#include "test_framework.hpp"
#include "echotrace/analysis.hpp"
#include "echotrace/document.hpp"
#include <vector>

TEST(AnalysisCompareAll2Sources) {
    echotrace::normalization::NormalizationOptions opts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int a = 1; return a;", echotrace::Language::Cpp, 5, 4, opts));
    sources.push_back(echotrace::parse_source("int b = 2; return b;", echotrace::Language::Cpp, 5, 4, opts));
    auto matches = echotrace::analysis::compare_all(sources, 1);
    ASSERT_EQ(matches.size(), static_cast<size_t>(1));
}

TEST(AnalysisCompareAll3Sources) {
    echotrace::normalization::NormalizationOptions opts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int a = 1;", echotrace::Language::Cpp, 5, 4, opts));
    sources.push_back(echotrace::parse_source("int b = 2;", echotrace::Language::Cpp, 5, 4, opts));
    sources.push_back(echotrace::parse_source("int c = 3;", echotrace::Language::Cpp, 5, 4, opts));
    auto matches = echotrace::analysis::compare_all(sources, 1);
    // 3 choose 2 = 3
    ASSERT_EQ(matches.size(), static_cast<size_t>(3));
}

TEST(AnalysisThresholdFiltering) {
    echotrace::normalization::NormalizationOptions nopts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int x = 0; return x;", echotrace::Language::Cpp, 5, 4, nopts));
    sources.push_back(echotrace::parse_source("int x = 0; return x;", echotrace::Language::Cpp, 5, 4, nopts));
    sources.push_back(echotrace::parse_source(
        "void insertion_sort(int* d, int n) { for (int i=1; i<n; ++i) { int k=d[i]; } }", echotrace::Language::Cpp, 5, 4, nopts));

    echotrace::analysis::AnalysisOptions opts;
    opts.k = 5;
    opts.window = 4;
    opts.threads = 1;
    opts.threshold = 50.0;

    auto result = echotrace::analysis::analyze(std::move(sources), opts);
    // Only the identical pair should survive threshold
    for (const auto& m : result.matches) {
        ASSERT_GE(m.similarity, 50.0);
    }
}

TEST(AnalysisTopNFiltering) {
    echotrace::normalization::NormalizationOptions nopts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int a = 1; return a;", echotrace::Language::Cpp, 5, 4, nopts));
    sources.push_back(echotrace::parse_source("int a = 1; return a;", echotrace::Language::Cpp, 5, 4, nopts));
    sources.push_back(echotrace::parse_source("int b = 2; return b;", echotrace::Language::Cpp, 5, 4, nopts));

    echotrace::analysis::AnalysisOptions opts;
    opts.k = 5;
    opts.window = 4;
    opts.threads = 1;
    opts.top_n = 1;

    auto result = echotrace::analysis::analyze(std::move(sources), opts);
    ASSERT_LE(result.matches.size(), static_cast<size_t>(1));
}

TEST(AnalysisResultsSortedDescending) {
    echotrace::normalization::NormalizationOptions nopts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int identical = 1; return identical;", echotrace::Language::Cpp, 5, 4, nopts));
    sources.push_back(echotrace::parse_source("int identical = 1; return identical;", echotrace::Language::Cpp, 5, 4, nopts));
    sources.push_back(echotrace::parse_source("void sort(int* d, int n) { for(int i=0;i<n;++i){} }", echotrace::Language::Cpp, 5, 4, nopts));

    echotrace::analysis::AnalysisOptions opts;
    opts.k = 5; opts.window = 4; opts.threads = 1;
    auto result = echotrace::analysis::analyze(std::move(sources), opts);
    for (size_t i = 1; i < result.matches.size(); ++i) {
        ASSERT_GE(result.matches[i-1].similarity, result.matches[i].similarity);
    }
}

TEST(AnalysisOneSourceEmpty) {
    echotrace::normalization::NormalizationOptions opts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int a = 1;", echotrace::Language::Cpp, 5, 4, opts));
    auto matches = echotrace::analysis::compare_all(sources, 1);
    ASSERT_TRUE(matches.empty());
}
