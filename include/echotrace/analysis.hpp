#ifndef ECHOTRACE_ANALYSIS_HPP
#define ECHOTRACE_ANALYSIS_HPP

#include "echotrace/document.hpp"
#include "echotrace/fs_traversal.hpp"
#include "echotrace/normalization.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace echotrace
{

/// Multi-file pairwise plagiarism analysis.
namespace analysis
{

/// A single pairwise comparison result. [lhs] and [rhs] index into the
/// AnalysisResult::sources vector (lhs < rhs).
struct Match
{
    std::size_t lhs = 0;
    std::size_t rhs = 0;
    double similarity = 0.0;
    std::size_t shared_fingerprints = 0;
};

/// Aggregate statistics over the full comparison set, computed BEFORE any
/// threshold/top-N filtering so the summary reflects the true distribution.
struct AnalysisSummary
{
    std::size_t files = 0;
    std::size_t comparisons = 0;
    std::size_t total_fingerprints = 0;
    double highest = 0.0;
    double average = 0.0;
    std::size_t matches_above_threshold = 0;
    normalization::NormalizationStats normalization{};
};

/// Tunables for an analysis run.
struct AnalysisOptions
{
    std::size_t k = 5;
    std::size_t window = 4;

    /// Worker threads for the comparison stage. 0 = hardware_concurrency()
    /// (clamped to >= 1). The parse stage is always single-threaded.
    std::size_t threads = 0;

    /// Minimum similarity (percent) for a match to appear in the results list.
    double threshold = 0.0;

    /// Keep only the top-N matches by similarity. 0 = keep all (above threshold).
    std::size_t top_n = 0;

    /// Normalisation passes applied during parsing.
    normalization::NormalizationOptions normalization{};
};

/// Full result of an analysis run.
struct AnalysisResult
{
    std::vector<ParsedSource> sources;
    std::vector<Match> matches;      // sorted by similarity descending
    AnalysisSummary summary;
    std::vector<std::string> warnings; // non-fatal issues (e.g. unreadable files)
};

/// Compute every unique pairwise match over [sources].
///
/// Complexity: O(n^2) comparisons, each O(min(|A|,|B|)). Each source is read
/// only (immutable), so concurrent comparisons never race; every task writes to
/// a distinct pre-assigned slot, making the collected results deterministic
/// regardless of thread scheduling.
///
/// [threads] == 0 selects std::thread::hardware_concurrency() (>= 1).
/// [threads] == 1 runs sequentially with no thread-pool overhead.
std::vector<Match> compare_all(const std::vector<ParsedSource>& sources,
                               std::size_t threads);

/// Analyse an already-parsed set of sources: compare, summarise, sort and
/// filter according to [options]. Parsing must already be complete.
AnalysisResult analyze(std::vector<ParsedSource> sources,
                       const AnalysisOptions& options);

/// Discover, read and parse every supported file under [root], then analyse.
/// File-read failures are recorded as warnings and the run continues with the
/// remaining files; if no file can be parsed, std::runtime_error is thrown.
AnalysisResult analyze_directory(const std::string& root,
                                 const AnalysisOptions& options);

} // namespace analysis

} // namespace echotrace

#endif // ECHOTRACE_ANALYSIS_HPP
