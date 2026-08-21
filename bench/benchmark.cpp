#include "echotrace/analysis.hpp"
#include "echotrace/document.hpp"
#include "echotrace/fingerprint.hpp"
#include "echotrace/hash.hpp"
#include "echotrace/lexer.hpp"
#include "echotrace/normalization.hpp"
#include "echotrace/similarity.hpp"
#include "echotrace/text.hpp"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{

using Clock = std::chrono::steady_clock;

struct Timer
{
    Clock::time_point start;
    Timer() : start(Clock::now()) {}
    double ms() const
    {
        return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    }
};

// Generate synthetic C++ source with controllable similarity.
std::string generate_source(std::size_t seed, std::size_t lines, double clone_fraction = 0.0,
                            const std::string* clone_from = nullptr)
{
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::ostringstream os;

    // Shared boilerplate if cloning
    std::size_t clone_lines = 0;
    if (clone_from && clone_fraction > 0.0)
    {
        clone_lines = static_cast<std::size_t>(static_cast<double>(lines) * clone_fraction);
        std::istringstream is(*clone_from);
        std::string line;
        for (std::size_t i = 0; i < clone_lines && std::getline(is, line); ++i)
        {
            os << line << '\n';
        }
    }

    static const char* patterns[] = {
        "int v%d = %d;\n",
        "if (v%d > %d) { v%d = v%d + %d; }\n",
        "for (int i = 0; i < %d; ++i) { v%d += i; }\n",
        "while (v%d > 0) { v%d--; }\n",
        "return v%d;\n",
        "double d%d = %d.%d;\n",
        "void func%d(int p) { int r = p * %d; }\n",
    };
    constexpr std::size_t npatterns = sizeof(patterns) / sizeof(patterns[0]);

    for (std::size_t i = clone_lines; i < lines; ++i)
    {
        auto pi = rng() % npatterns;
        char buf[256];
        std::snprintf(buf, sizeof(buf), patterns[pi],
                      static_cast<int>(rng() % 100),
                      static_cast<int>(rng() % 1000),
                      static_cast<int>(rng() % 100),
                      static_cast<int>(rng() % 100),
                      static_cast<int>(rng() % 50));
        os << buf;
    }
    return os.str();
}

void bench_stage(const char* name, double ms, std::size_t items = 0)
{
    std::cout << std::left << std::setw(30) << name
              << std::right << std::setw(10) << std::fixed << std::setprecision(2) << ms << " ms";
    if (items > 0)
    {
        std::cout << "  (" << items << " items)";
    }
    std::cout << '\n';
}

void run_benchmark(std::size_t n_files, std::size_t lines_per_file, std::size_t threads)
{
    std::cout << "\n=== Benchmark: " << n_files << " files, "
              << lines_per_file << " lines each, " << threads << " thread(s) ===\n\n";

    // Generate sources
    Timer gen_timer;
    std::vector<std::string> raw_sources;
    raw_sources.reserve(n_files);

    // First file is the "original"
    raw_sources.push_back(generate_source(0, lines_per_file));

    // Some files are clones (first 20% are 80% similar), rest are unique
    std::size_t n_clones = n_files / 5;
    for (std::size_t i = 1; i < n_files; ++i)
    {
        if (i <= n_clones)
        {
            raw_sources.push_back(generate_source(i, lines_per_file, 0.8, &raw_sources[0]));
        }
        else
        {
            raw_sources.push_back(generate_source(i * 1000, lines_per_file));
        }
    }
    bench_stage("Source generation", gen_timer.ms(), n_files);

    // Parse stage
    Timer parse_timer;
    echotrace::normalization::NormalizationOptions opts{};
    std::vector<echotrace::ParsedSource> sources;
    sources.reserve(n_files);
    for (std::size_t i = 0; i < n_files; ++i)
    {
        auto parsed = echotrace::parse_source(raw_sources[i], 5, 4, opts);
        parsed.path = "file_" + std::to_string(i) + ".cpp";
        sources.push_back(std::move(parsed));
    }
    bench_stage("Parsing + normalization", parse_timer.ms(), n_files);

    // Total tokens/fingerprints
    std::size_t total_tokens = 0, total_fps = 0;
    for (const auto& s : sources)
    {
        total_tokens += s.tokens.size();
        total_fps += s.fingerprint.size();
    }
    std::cout << "  Total tokens:       " << total_tokens << '\n';
    std::cout << "  Total fingerprints: " << total_fps << '\n';

    // Comparison stage
    std::size_t n_pairs = n_files * (n_files - 1) / 2;
    Timer cmp_timer;
    auto matches = echotrace::analysis::compare_all(sources, threads);
    bench_stage("Pairwise comparison", cmp_timer.ms(), n_pairs);

    // Count matches above thresholds
    std::size_t above_50 = 0, above_80 = 0;
    for (const auto& m : matches)
    {
        if (m.similarity >= 50.0) ++above_50;
        if (m.similarity >= 80.0) ++above_80;
    }
    std::cout << "  Pairs >= 50%: " << above_50 << '\n';
    std::cout << "  Pairs >= 80%: " << above_80 << '\n';

    // Total
    double total = gen_timer.ms() + parse_timer.ms() + cmp_timer.ms();
    bench_stage("TOTAL", total);
}

} // namespace

int main()
{
    std::cout << "EchoTrace Benchmark Suite\n";
    std::cout << "========================\n";

    // Small datasets
    run_benchmark(10, 50, 1);
    run_benchmark(10, 50, 4);

    // Medium datasets
    run_benchmark(50, 100, 1);
    run_benchmark(50, 100, 4);

    // Larger datasets
    run_benchmark(100, 100, 1);
    run_benchmark(100, 100, 4);

    // Scale test
    run_benchmark(200, 50, 4);

    std::cout << "\nBenchmark complete.\n";
    return 0;
}
