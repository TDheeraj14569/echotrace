#include "test_framework.hpp"
#include "echotrace/reporting.hpp"
#include "echotrace/analysis.hpp"
#include "echotrace/document.hpp"
#include <string>
#include <stdexcept>

// Helper to create a minimal AnalysisResult for testing report renderers.
static echotrace::analysis::AnalysisResult make_test_result() {
    echotrace::normalization::NormalizationOptions nopts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.push_back(echotrace::parse_source("int x = 1; return x;", echotrace::Language::Cpp, 5, 4, nopts));
    sources[0].path = "file_a.cpp";
    sources.push_back(echotrace::parse_source("int y = 2; return y;", echotrace::Language::Cpp, 5, 4, nopts));
    sources[1].path = "file_b.cpp";

    echotrace::analysis::AnalysisOptions opts;
    opts.k = 5; opts.window = 4; opts.threads = 1;
    return echotrace::analysis::analyze(std::move(sources), opts);
}

static echotrace::reporting::ReportMeta make_test_meta() {
    echotrace::reporting::ReportMeta meta;
    meta.root = "test_dir";
    meta.timestamp = "2026-01-01T00:00:00";
    meta.duration_ms = 42.0;
    meta.k = 5;
    meta.window = 4;
    meta.threads = 1;
    return meta;
}

TEST(ReportingRenderText) {
    auto result = make_test_result();
    auto meta = make_test_meta();
    auto out = echotrace::reporting::render_text(result, meta);
    ASSERT_FALSE(out.empty());
    ASSERT_TRUE(out.find("EchoTrace") != std::string::npos);
}

TEST(ReportingRenderCsv) {
    auto result = make_test_result();
    auto meta = make_test_meta();
    auto out = echotrace::reporting::render_csv(result, meta);
    ASSERT_FALSE(out.empty());
    ASSERT_TRUE(out.find("rank") != std::string::npos);
}

TEST(ReportingRenderJson) {
    auto result = make_test_result();
    auto meta = make_test_meta();
    auto out = echotrace::reporting::render_json(result, meta);
    ASSERT_FALSE(out.empty());
    ASSERT_TRUE(out.front() == '{');
}

TEST(ReportingRenderHtml) {
    auto result = make_test_result();
    auto meta = make_test_meta();
    auto out = echotrace::reporting::render_html(result, meta);
    ASSERT_FALSE(out.empty());
    ASSERT_TRUE(out.find("<!DOCTYPE html>") != std::string::npos);
}

TEST(ReportingParseFormat) {
    ASSERT_EQ(echotrace::reporting::parse_format("text"), echotrace::reporting::ReportFormat::Text);
    ASSERT_EQ(echotrace::reporting::parse_format("csv"), echotrace::reporting::ReportFormat::Csv);
    ASSERT_EQ(echotrace::reporting::parse_format("json"), echotrace::reporting::ReportFormat::Json);
    ASSERT_EQ(echotrace::reporting::parse_format("html"), echotrace::reporting::ReportFormat::Html);
    ASSERT_EQ(echotrace::reporting::parse_format("HTML"), echotrace::reporting::ReportFormat::Html);
}

TEST(ReportingParseFormatThrows) {
    ASSERT_THROW(echotrace::reporting::parse_format("xml"), std::invalid_argument);
    ASSERT_THROW(echotrace::reporting::parse_format("unknown"), std::invalid_argument);
}

TEST(ReportingEmptyResults) {
    echotrace::analysis::AnalysisResult result;
    auto meta = make_test_meta();
    auto text_out = echotrace::reporting::render_text(result, meta);
    ASSERT_FALSE(text_out.empty());
    auto csv_out = echotrace::reporting::render_csv(result, meta);
    ASSERT_FALSE(csv_out.empty());
    auto json_out = echotrace::reporting::render_json(result, meta);
    ASSERT_FALSE(json_out.empty());
}

TEST(ReportingRenderDispatch) {
    auto result = make_test_result();
    auto meta = make_test_meta();
    auto out = echotrace::reporting::render(result, meta, echotrace::reporting::ReportFormat::Text);
    ASSERT_FALSE(out.empty());
}
