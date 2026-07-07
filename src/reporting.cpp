#include "echotrace/reporting.hpp"

#include "echotrace/fragments.hpp"
#include "echotrace/text.hpp"
#include "echotrace/version.hpp"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace echotrace
{

namespace reporting
{

namespace
{

// Fixed-point double -> string with [decimals] digits after the point.
std::string fmt_double(double v, int decimals = 2)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
    return std::string(buf);
}

// Trim a source fragment to a single readable line for inline display.
std::string oneline(std::string_view s, std::size_t max_len = 80)
{
    std::string out;
    out.reserve(s.size());
    bool was_space = false;
    for (char c : s)
    {
        if (c == '\n' || c == '\r' || c == '\t')
        {
            c = ' ';
        }
        if (c == ' ')
        {
            if (was_space)
            {
                continue;
            }
            was_space = true;
        }
        else
        {
            was_space = false;
        }
        out.push_back(c);
    }
    if (out.size() > max_len)
    {
        out.resize(max_len);
        if (max_len >= 3)
        {
            out[max_len - 3] = '.';
            out[max_len - 2] = '.';
            out[max_len - 1] = '.';
        }
    }
    return out;
}

const std::string& display(const AnalysisResult& r, std::size_t idx)
{
    return r.sources.at(idx).path;
}

} // namespace

ReportFormat parse_format(const std::string& name)
{
    const auto lower = text::to_lower(name);
    if (lower == "text" || lower == "txt")
    {
        return ReportFormat::Text;
    }
    if (lower == "csv")
    {
        return ReportFormat::Csv;
    }
    if (lower == "json")
    {
        return ReportFormat::Json;
    }
    if (lower == "html" || lower == "htm")
    {
        return ReportFormat::Html;
    }
    throw std::invalid_argument("unknown report format '" + name +
                                "' (expected text, csv, json or html)");
}

// --------------------------------------------------------------------------
// Text
// --------------------------------------------------------------------------
std::string render_text(const AnalysisResult& result, const ReportMeta& meta)
{
    std::ostringstream os;
    os << "EchoTrace Analysis Report\n";
    os << "=========================\n\n";
    os << "Root:            " << meta.root << "\n";
    os << "Generated:       " << meta.timestamp << "\n";
    os << "Duration:        " << fmt_double(meta.duration_ms, 2) << " ms\n";
    os << "k-gram size:     " << meta.k << "\n";
    os << "Winnow window:   " << meta.window << "\n";
    os << "Threads:         " << (meta.threads == 0 ? std::string("1 (sequential)")
                                                    : std::to_string(meta.threads))
       << "\n\n";

    os << "Summary\n";
    os << "-------\n";
    os << "Files:                  " << result.summary.files << "\n";
    os << "Comparisons:            " << result.summary.comparisons << "\n";
    os << "Highest similarity:     " << fmt_double(result.summary.highest) << "%\n";
    os << "Average similarity:     " << fmt_double(result.summary.average) << "%\n";
    os << "Matches (>= " << fmt_double(meta.threshold) << "%):       "
       << result.summary.matches_above_threshold << "\n";
    os << "Total fingerprints:     " << result.summary.total_fingerprints << "\n\n";

    os << "Normalization\n";
    os << "-------------\n";
    os << "Parentheses collapsed:  " << result.summary.normalization.parens_collapsed << "\n\n";

    os << "Ranked Matches\n";
    os << "--------------\n";
    if (result.matches.empty())
    {
        os << "(no matches above threshold)\n";
    }
    else
    {
        os << "rank  similarity   file_a                                 file_b                                 shared\n";
        std::size_t rank = 1;
        for (const auto& m : result.matches)
        {
            os << std::right << std::setw(4) << rank++ << "   "
               << std::right << std::setw(8) << fmt_double(m.similarity) << "%   "
               << std::left << std::setw(38) << display(result, m.lhs) << "  "
               << std::left << std::setw(38) << display(result, m.rhs) << "  "
               << std::right << std::setw(6) << m.shared_fingerprints << "\n";
        }
    }

    if (!result.warnings.empty())
    {
        os << "\nWarnings\n";
        os << "--------\n";
        for (const auto& w : result.warnings)
        {
            os << "- " << w << "\n";
        }
    }

    return os.str();
}

// --------------------------------------------------------------------------
// CSV
// --------------------------------------------------------------------------
std::string render_csv(const AnalysisResult& result, const ReportMeta& /*meta*/)
{
    std::ostringstream os;
    os << "rank,file_a,file_b,similarity_percent,shared_fingerprints\n";
    std::size_t rank = 1;
    for (const auto& m : result.matches)
    {
        os << rank++ << ','
           << text::csv_escape(display(result, m.lhs)) << ','
           << text::csv_escape(display(result, m.rhs)) << ','
           << fmt_double(m.similarity) << ','
           << m.shared_fingerprints << '\n';
    }
    return os.str();
}

// --------------------------------------------------------------------------
// JSON
// --------------------------------------------------------------------------
std::string render_json(const AnalysisResult& result,
                        const ReportMeta& meta,
                        std::size_t fragment_limit)
{
    std::ostringstream os;

    os << "{\n";
    os << "  \"tool\": \"EchoTrace\",\n";
    os << "  \"version\": \"" << kVersion << "\",\n";
    os << "  \"root\": \"" << text::json_escape(meta.root) << "\",\n";
    os << "  \"timestamp\": \"" << text::json_escape(meta.timestamp) << "\",\n";
    os << "  \"duration_ms\": " << fmt_double(meta.duration_ms) << ",\n";
    os << "  \"parameters\": {\n";
    os << "    \"k\": " << meta.k << ",\n";
    os << "    \"window\": " << meta.window << ",\n";
    os << "    \"threads\": " << meta.threads << ",\n";
    os << "    \"threshold\": " << fmt_double(meta.threshold) << ",\n";
    os << "    \"top_n\": " << meta.top_n << "\n";
    os << "  },\n";
    os << "  \"summary\": {\n";
    os << "    \"files\": " << result.summary.files << ",\n";
    os << "    \"comparisons\": " << result.summary.comparisons << ",\n";
    os << "    \"highest_similarity\": " << fmt_double(result.summary.highest) << ",\n";
    os << "    \"average_similarity\": " << fmt_double(result.summary.average) << ",\n";
    os << "    \"matches_above_threshold\": " << result.summary.matches_above_threshold << ",\n";
    os << "    \"total_fingerprints\": " << result.summary.total_fingerprints << "\n";
    os << "  },\n";
    os << "  \"normalization\": {\n";
    os << "    \"parens_collapsed\": " << result.summary.normalization.parens_collapsed << "\n";
    os << "  },\n";

    os << "  \"warnings\": [";
    for (std::size_t i = 0; i < result.warnings.size(); ++i)
    {
        if (i != 0)
        {
            os << ", ";
        }
        os << '"' << text::json_escape(result.warnings[i]) << '"';
    }
    os << "],\n";

    os << "  \"matches\": [\n";
    const std::size_t with_fragments = std::min(fragment_limit, result.matches.size());
    for (std::size_t idx = 0; idx < result.matches.size(); ++idx)
    {
        const auto& m = result.matches[idx];
        os << "    {\n";
        os << "      \"rank\": " << (idx + 1) << ",\n";
        os << "      \"file_a\": \"" << text::json_escape(display(result, m.lhs)) << "\",\n";
        os << "      \"file_b\": \"" << text::json_escape(display(result, m.rhs)) << "\",\n";
        os << "      \"similarity\": " << fmt_double(m.similarity) << ",\n";
        os << "      \"shared_fingerprints\": " << m.shared_fingerprints;

        if (idx < with_fragments)
        {
            auto frags = fragments::matched_fragments(
                result.sources[m.lhs], result.sources[m.rhs], meta.k);
            os << ",\n      \"fragments\": [";
            for (std::size_t fi = 0; fi < frags.size(); ++fi)
            {
                if (fi != 0)
                {
                    os << ", ";
                }
                const auto& f = frags[fi];
                os << "{\"a_offset\":" << f.a_offset << ",\"a_length\":" << f.a_length
                   << ",\"a_text\":\"" << text::json_escape(f.a_text) << "\""
                   << ",\"b_offset\":" << f.b_offset << ",\"b_length\":" << f.b_length
                   << ",\"b_text\":\"" << text::json_escape(f.b_text) << "\"}";
            }
            os << "]";
        }
        os << "\n    }";
        if (idx + 1 < result.matches.size())
        {
            os << ",";
        }
        os << "\n";
    }
    os << "  ]\n";
    os << "}\n";
    return os.str();
}

// --------------------------------------------------------------------------
// HTML
// --------------------------------------------------------------------------
std::string render_html(const AnalysisResult& result,
                        const ReportMeta& meta,
                        std::size_t fragment_limit)
{
    std::ostringstream os;
    os << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n";
    os << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    os << "<title>EchoTrace Report</title>\n<style>\n";
    os << "body{font-family:-apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif;"
       << "margin:0;background:#f6f7fb;color:#1f2430;}\n";
    os << ".wrap{max-width:1100px;margin:0 auto;padding:24px;}\n";
    os << "h1{font-size:1.5rem;margin:0 0 4px;}\n";
    os << ".meta{color:#5b6472;font-size:.85rem;margin-bottom:18px;}\n";
    os << ".cards{display:flex;flex-wrap:wrap;gap:12px;margin:14px 0 22px;}\n";
    os << ".card{background:#fff;border:1px solid #e3e6ef;border-radius:10px;padding:12px 16px;"
       << "min-width:130px;box-shadow:0 1px 2px rgba(20,30,60,.04);}\n";
    os << ".card .v{font-size:1.35rem;font-weight:700;}\n";
    os << ".card .l{font-size:.72rem;text-transform:uppercase;letter-spacing:.04em;color:#7a8493;}\n";
    os << "table{border-collapse:collapse;width:100%;background:#fff;border-radius:10px;"
       << "overflow:hidden;border:1px solid #e3e6ef;}\n";
    os << "th,td{padding:9px 12px;text-align:left;border-bottom:1px solid #eef0f6;font-size:.9rem;}\n";
    os << "th{background:#f0f2f8;font-size:.72rem;text-transform:uppercase;letter-spacing:.04em;}\n";
    os << "tr:hover td{background:#fafbff;}\n";
    os << ".pct{font-variant-numeric:tabular-nums;font-weight:600;}\n";
    os << ".hi{color:#c0392b;}.mid{color:#b8860b;}.lo{color:#3a7d34;}\n";
    os << ".frag{background:#0f1320;color:#e8eaf0;border-radius:8px;padding:10px 12px;"
       << "font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;font-size:.82rem;"
       << "white-space:pre-wrap;margin:8px 0;overflow-x:auto;}\n";
    os << ".pair{margin:18px 0;padding:14px;background:#fff;border:1px solid #e3e6ef;border-radius:10px;}\n";
    os << ".pair h3{margin:0 0 6px;font-size:.95rem;}\n";
    os << ".lab{color:#7a8493;font-size:.72rem;text-transform:uppercase;letter-spacing:.04em;}\n";
    os << "</style>\n</head>\n<body><div class=\"wrap\">\n";
    os << "<h1>EchoTrace Plagiarism Report</h1>\n";
    os << "<div class=\"meta\">Root: " << text::html_escape(meta.root)
       << " &middot; " << text::html_escape(meta.timestamp)
       << " &middot; " << fmt_double(meta.duration_ms) << " ms"
       << " &middot; k=" << meta.k << " w=" << meta.window
       << " &middot; threads=" << (meta.threads == 0 ? 1 : meta.threads) << "</div>\n";

    const auto cls = [](double s)
    {
        return s >= 70.0 ? "hi" : (s >= 30.0 ? "mid" : "lo");
    };

    os << "<div class=\"cards\">\n";
    auto card = [&](const char* label, const std::string& value, const char* extra = "")
    {
        os << "<div class=\"card\"><div class=\"l\">" << label << "</div>"
           << "<div class=\"v " << extra << "\">" << value << "</div></div>\n";
    };
    card("Files", std::to_string(result.summary.files));
    card("Comparisons", std::to_string(result.summary.comparisons));
    card("Highest", fmt_double(result.summary.highest) + "%", cls(result.summary.highest));
    card("Average", fmt_double(result.summary.average) + "%", cls(result.summary.average));
    card("Matches", std::to_string(result.summary.matches_above_threshold));
    card("Parens collapsed", std::to_string(result.summary.normalization.parens_collapsed));
    os << "</div>\n";

    os << "<h2>Ranked Matches</h2>\n<table>\n";
    os << "<thead><tr><th>#</th><th>Similarity</th><th>File A</th><th>File B</th>"
       << "<th>Shared</th></tr></thead>\n<tbody>\n";
    std::size_t rank = 1;
    for (const auto& m : result.matches)
    {
        os << "<tr><td>" << rank++ << "</td>"
           << "<td class=\"pct " << cls(m.similarity) << "\">"
           << fmt_double(m.similarity) << "%</td>"
           << "<td>" << text::html_escape(display(result, m.lhs)) << "</td>"
           << "<td>" << text::html_escape(display(result, m.rhs)) << "</td>"
           << "<td>" << m.shared_fingerprints << "</td></tr>\n";
    }
    os << "</tbody></table>\n";

    const std::size_t with_fragments = std::min(fragment_limit, result.matches.size());
    if (with_fragments > 0)
    {
        os << "<h2>Top Matched Fragments</h2>\n";
        for (std::size_t idx = 0; idx < with_fragments; ++idx)
        {
            const auto& m = result.matches[idx];
            auto frags = fragments::matched_fragments(
                result.sources[m.lhs], result.sources[m.rhs], meta.k);
            if (frags.empty())
            {
                continue;
            }
            os << "<div class=\"pair\"><h3>#" << (idx + 1) << " "
               << text::html_escape(display(result, m.lhs)) << " &harr; "
               << text::html_escape(display(result, m.rhs))
               << " &mdash; " << fmt_double(m.similarity) << "%</h3>\n";
            for (const auto& f : frags)
            {
                os << "<div class=\"lab\">A</div><div class=\"frag\">"
                   << text::html_escape(oneline(f.a_text)) << "</div>\n";
                os << "<div class=\"lab\">B</div><div class=\"frag\">"
                   << text::html_escape(oneline(f.b_text)) << "</div>\n";
            }
            os << "</div>\n";
        }
    }

    if (!result.warnings.empty())
    {
        os << "<h2>Warnings</h2><ul>\n";
        for (const auto& w : result.warnings)
        {
            os << "<li>" << text::html_escape(w) << "</li>\n";
        }
        os << "</ul>\n";
    }

    os << "</div></body></html>\n";
    return os.str();
}

std::string render(const AnalysisResult& result,
                   const ReportMeta& meta,
                   ReportFormat format,
                   std::size_t fragment_limit)
{
    switch (format)
    {
    case ReportFormat::Text: return render_text(result, meta);
    case ReportFormat::Csv:  return render_csv(result, meta);
    case ReportFormat::Json: return render_json(result, meta, fragment_limit);
    case ReportFormat::Html: return render_html(result, meta, fragment_limit);
    }
    return {};
}

} // namespace reporting

} // namespace echotrace
