#ifndef ECHOTRACE_REPORTING_HPP
#define ECHOTRACE_REPORTING_HPP

#include "echotrace/analysis.hpp"

#include <cstddef>
#include <string>

namespace echotrace
{

/// Report rendering for analysis results across multiple formats.
namespace reporting
{

using analysis::AnalysisResult;

/// Output format selector.
enum class ReportFormat
{
    Text,
    Csv,
    Json,
    Html
};

/// Parse a format name (case-insensitive: text/csv/json/html) into a
/// ReportFormat. Throws std::invalid_argument on an unknown name.
ReportFormat parse_format(const std::string& name);

/// Run metadata captured by the caller and embedded in every report.
struct ReportMeta
{
    std::string root;          // scanned root or "two-file"
    std::string timestamp;     // ISO-8601 local time
    double duration_ms = 0.0;  // total wall-clock processing time
    std::size_t k = 5;
    std::size_t window = 4;
    std::size_t threads = 0;   // 0 == sequential
    double threshold = 0.0;    // percent
    std::size_t top_n = 0;     // 0 == unlimited
};

/// Human-readable text report: summary block + ranked match table.
std::string render_text(const AnalysisResult& result, const ReportMeta& meta);

/// RFC-4180 CSV of the ranked matches (rank, file_a, file_b, similarity%,
/// shared_fingerprints). One row per match, header included.
std::string render_csv(const AnalysisResult& result, const ReportMeta& meta);

/// Structured JSON document with summary, warnings, ranked matches and (for the
/// top [fragment_limit] matches) matched fragments.
std::string render_json(const AnalysisResult& result,
                        const ReportMeta& meta,
                        std::size_t fragment_limit = 10);

/// Self-contained HTML report (inline CSS, no external resources) with the
/// summary, ranked matches and matched-fragment snippets for the top
/// [fragment_limit] matches.
std::string render_html(const AnalysisResult& result,
                        const ReportMeta& meta,
                        std::size_t fragment_limit = 10);

/// Dispatch to the renderer for [format].
std::string render(const AnalysisResult& result,
                   const ReportMeta& meta,
                   ReportFormat format,
                   std::size_t fragment_limit = 10);

} // namespace reporting

} // namespace echotrace

#endif // ECHOTRACE_REPORTING_HPP
