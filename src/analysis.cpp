#include "echotrace/analysis.hpp"

#include "echotrace/similarity.hpp"
#include "echotrace/text.hpp"
#include "echotrace/thread_pool.hpp"

#include <algorithm>
#include <future>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <utility>

namespace echotrace
{

namespace analysis
{

namespace
{

// Resolve the worker count: 0 -> hardware, clamped to [1, max].
std::size_t resolve_threads(std::size_t requested)
{
    if (requested != 0)
    {
        return requested;
    }
    const auto hw = std::thread::hardware_concurrency();
    return hw == 0 ? 1 : hw;
}

} // namespace

std::vector<Match> compare_all(const std::vector<ParsedSource>& sources,
                               std::size_t threads)
{
    const auto n = sources.size();
    std::vector<Match> matches;

    if (n < 2)
    {
        return matches;
    }

    // Enumerate the n*(n-1)/2 unique pairs once, in a stable order.
    std::vector<std::pair<std::size_t, std::size_t>> pairs;
    const std::size_t total = n * (n - 1) / 2;
    pairs.reserve(total);
    for (std::size_t i = 0; i + 1 < n; ++i)
    {
        for (std::size_t j = i + 1; j < n; ++j)
        {
            pairs.emplace_back(i, j);
        }
    }

    matches.resize(total);

    const std::size_t workers = resolve_threads(threads);

    if (workers == 1 || total <= 1)
    {
        // Sequential path: no thread-pool overhead for tiny inputs.
        for (std::size_t t = 0; t < total; ++t)
        {
            const auto [i, j] = pairs[t];
            const auto d = sim::jaccard_detail(sources[i].fingerprint, sources[j].fingerprint);
            matches[t] = Match{i, j, d.percent, d.intersection};
        }
        return matches;
    }

    // Parallel path. Each task writes to its OWN slot (matches[t]); since every
    // task owns a distinct index, there is no shared mutable state and no lock.
    // Sources are read-only and therefore safe to share across workers.
    ThreadPool pool(workers);
    std::vector<std::future<void>> futures;
    futures.reserve(total);

    for (std::size_t t = 0; t < total; ++t)
    {
        const auto i = pairs[t].first;
        const auto j = pairs[t].second;
        futures.push_back(pool.submit(
            [&sources, &matches, t, i, j]()
            {
                const auto d = sim::jaccard_detail(sources[i].fingerprint, sources[j].fingerprint);
                matches[t] = Match{i, j, d.percent, d.intersection};
            }));
    }

    // Re-await every task; any thrown exception propagates here.
    for (auto& f : futures)
    {
        f.get();
    }

    return matches;
}

AnalysisResult analyze(std::vector<ParsedSource> sources,
                       const AnalysisOptions& options)
{
    AnalysisResult result;
    result.sources = std::move(sources);
    auto& srcs = result.sources;

    // Aggregate normalisation statistics across all sources.
    for (const auto& s : srcs)
    {
        result.summary.normalization.parens_collapsed += s.norm_stats.parens_collapsed;
        result.summary.total_fingerprints += s.fingerprint.size();
    }

    // Run the (optionally parallel) comparison stage.
    auto all = compare_all(srcs, options.threads);

    // Summary reflects the FULL distribution, before filtering.
    result.summary.files = srcs.size();
    result.summary.comparisons = all.size();
    if (!all.empty())
    {
        double sum = 0.0;
        double highest = 0.0;
        for (const auto& m : all)
        {
            sum += m.similarity;
            if (m.similarity > highest)
            {
                highest = m.similarity;
            }
        }
        result.summary.average = sum / static_cast<double>(all.size());
        result.summary.highest = highest;
    }

    // Apply threshold and top-N filters, then sort by similarity descending.
    // Stable ordering is preserved across equal similarities by (lhs, rhs).
    std::vector<Match> kept;
    kept.reserve(all.size());
    for (const auto& m : all)
    {
        if (m.similarity + 1e-9 >= options.threshold)
        {
            kept.push_back(m);
        }
    }

    std::stable_sort(kept.begin(), kept.end(),
                     [](const Match& a, const Match& b)
                     {
                         if (a.similarity != b.similarity)
                         {
                             return a.similarity > b.similarity;
                         }
                         if (a.lhs != b.lhs)
                         {
                             return a.lhs < b.lhs;
                         }
                         return a.rhs < b.rhs;
                     });

    result.summary.matches_above_threshold = kept.size();

    if (options.top_n != 0 && kept.size() > options.top_n)
    {
        kept.resize(options.top_n);
    }

    result.matches = std::move(kept);
    return result;
}

AnalysisResult analyze_directory(const std::string& root,
                                 const AnalysisOptions& options)
{
    const auto files = fs::discover_sources(root);

    std::vector<ParsedSource> sources;
    sources.reserve(files.size());
    std::vector<std::string> warnings;

    // Parse stage: single-threaded, exactly once per file.
    for (const auto& f : files)
    {
        std::string content;
        try
        {
            content = io::read_file(f.path);
        }
        catch (const std::exception& ex)
        {
            warnings.push_back(std::string("skipped '") + f.display + "': " + ex.what());
            continue;
        }
        auto parsed = parse_source(content, options.k, options.window, options.normalization);
        parsed.path = f.display;
        sources.push_back(std::move(parsed));
    }

    if (sources.empty())
    {
        throw std::runtime_error("no parsable C++ source files found under '" + root + "'");
    }

    auto result = analyze(std::move(sources), options);
    result.warnings = std::move(warnings);
    return result;
}

} // namespace analysis

} // namespace echotrace
