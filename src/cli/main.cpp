// EchoTrace command-line interface.
//
// Supports the original two-file workflow unchanged (apart from product
// branding) plus the Phase 1+ directory analysis mode with filters and
// multi-format reporting.

#include "echotrace/analysis.hpp"
#include "echotrace/document.hpp"
#include "echotrace/normalization.hpp"
#include "echotrace/reporting.hpp"
#include "echotrace/similarity.hpp"
#include "echotrace/text.hpp"
#include "echotrace/version.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{

namespace stdfs = std::filesystem;

struct Cli
{
    bool help = false;
    bool version = false;
    bool self_test = false;
    std::size_t k = 5;
    std::size_t window = 4;
    std::size_t threads = 0;        // 0 == auto
    double threshold = 0.0;         // percent
    std::size_t top_n = 0;          // 0 == all
    bool no_normalize = false;
    std::string format = "text";
    std::string output;             // empty == stdout
    std::vector<std::string> paths;
};

bool starts_with(std::string_view s, std::string_view prefix)
{
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

std::string str(std::string_view s)
{
    return std::string(s.data(), s.size());
}

std::size_t parse_size(std::string_view raw, std::string_view opt)
{
    std::string text = str(raw);
    std::size_t consumed = 0;
    unsigned long long value = 0;

    try
    {
        value = std::stoull(text, &consumed, 10);
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument(str(opt) + " expects a positive integer");
    }

    if (consumed != text.size() || value == 0)
    {
        throw std::invalid_argument(str(opt) + " expects a positive integer");
    }
    if (value > static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max()))
    {
        throw std::out_of_range(str(opt) + " is too large for this build");
    }

    return static_cast<std::size_t>(value);
}

double parse_percent(std::string_view raw, std::string_view opt)
{
    std::string text = str(raw);
    std::size_t consumed = 0;
    double value = 0.0;

    try
    {
        value = std::stod(text, &consumed);
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument(str(opt) + " expects a number in [0, 100]");
    }

    if (consumed != text.size())
    {
        throw std::invalid_argument(str(opt) + " expects a number in [0, 100]");
    }
    if (value < 0.0 || value > 100.0)
    {
        throw std::invalid_argument(str(opt) + " must be in [0, 100]");
    }
    return value;
}

Cli parse_args(int argc, char* argv[])
{
    Cli cli;

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg(argv[i]);

        auto take_next = [&](std::string_view opt) -> std::string_view
        {
            if (i + 1 >= argc)
            {
                throw std::invalid_argument(str(opt) + " requires a value");
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h")
        {
            cli.help = true;
        }
        else if (arg == "--version")
        {
            cli.version = true;
        }
        else if (arg == "--self-test")
        {
            cli.self_test = true;
        }
        else if (arg == "--k" || arg == "-k")
        {
            cli.k = parse_size(take_next(arg), arg);
        }
        else if (starts_with(arg, "--k="))
        {
            cli.k = parse_size(arg.substr(4), "--k");
        }
        else if (arg == "--w" || arg == "-w")
        {
            cli.window = parse_size(take_next(arg), arg);
        }
        else if (starts_with(arg, "--w="))
        {
            cli.window = parse_size(arg.substr(4), "--w");
        }
        else if (arg == "--threads" || arg == "-j")
        {
            cli.threads = parse_size(take_next(arg), arg);
        }
        else if (starts_with(arg, "--threads="))
        {
            cli.threads = parse_size(arg.substr(10), "--threads");
        }
        else if (arg == "--threshold" || arg == "--min")
        {
            cli.threshold = parse_percent(take_next(arg), arg);
        }
        else if (starts_with(arg, "--threshold="))
        {
            cli.threshold = parse_percent(arg.substr(12), "--threshold");
        }
        else if (arg == "--top" || arg == "--top-n")
        {
            cli.top_n = parse_size(take_next(arg), arg);
        }
        else if (starts_with(arg, "--top="))
        {
            cli.top_n = parse_size(arg.substr(6), "--top");
        }
        else if (arg == "--format")
        {
            cli.format = str(take_next(arg));
        }
        else if (starts_with(arg, "--format="))
        {
            cli.format = str(arg.substr(9));
        }
        else if (arg == "--output" || arg == "-o")
        {
            cli.output = str(take_next(arg));
        }
        else if (starts_with(arg, "--output="))
        {
            cli.output = str(arg.substr(9));
        }
        else if (arg == "--no-normalize")
        {
            cli.no_normalize = true;
        }
        else if (arg == "--dir")
        {
            cli.paths.push_back(str(take_next(arg)));
        }
        else if (!arg.empty() && arg[0] == '-')
        {
            throw std::invalid_argument("unknown option '" + str(arg) + "'");
        }
        else
        {
            cli.paths.emplace_back(arg.data(), arg.size());
        }
    }

    if (!cli.help && cli.self_test && !cli.paths.empty())
    {
        throw std::invalid_argument("--self-test does not accept file paths");
    }

    return cli;
}

void print_usage(std::ostream& out, std::string_view exe)
{
    out << "EchoTrace " << echotrace::kVersion << " \xE2\x80\x94 C++ plagiarism detection\n"
        << "\nUsage:\n"
        << "  " << exe << " <directory>            Analyse every source file in a directory\n"
        << "  " << exe << " <left.cpp> <right.cpp> Compare two files (legacy two-file mode)\n"
        << "  " << exe << " --self-test\n"
        << "  " << exe << " --help\n\n"
        << "Analysis options:\n"
        << "  -k, --k N              token k-gram size (default 5)\n"
        << "  -w, --w N              winnowing window size (default 4)\n"
        << "  -j, --threads N        comparison worker threads (default: auto)\n"
        << "  --threshold PCT, --min PCT   minimum similarity to report (default 0)\n"
        << "  --top N                keep only the top N matches (default: all)\n"
        << "  --no-normalize         disable redundant-paren normalisation\n\n"
        << "Output options:\n"
        << "  --format text|csv|json|html   report format (default: text)\n"
        << "  -o, --output FILE      write report to FILE instead of stdout\n";
}

// ISO-8601 local timestamp, e.g. 2026-07-07T14:03:11.
std::string now_iso()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return os.str();
}

echotrace::normalization::NormalizationOptions norm_options(const Cli& cli)
{
    echotrace::normalization::NormalizationOptions opts;
    if (cli.no_normalize)
    {
        opts.collapse_redundant_parens = false;
    }
    return opts;
}

// ---- legacy two-file comparison ------------------------------------------
void run_case(std::string_view name,
              std::string_view lhs,
              std::string_view rhs,
              std::string_view expectation,
              std::size_t k,
              std::size_t window)
{
    const auto score = echotrace::sim::compare(lhs, rhs, k, window);

    std::cout << std::left << std::setw(22) << name
              << " similarity: " << std::right << std::fixed << std::setprecision(2)
              << std::setw(6) << score << "%  " << expectation << '\n';
}

void run_self_test(std::size_t k, std::size_t window)
{
    const auto exact_a = R"cpp(
        int add(int a, int b)
        {
            int total = a + b;
            return total;
        }

        int main()
        {
            int x = 4;
            int y = 7;
            return add(x, y);
        }
    )cpp";

    const auto exact_b = R"cpp(
        int add(int a, int b)
        {
            int total = a + b;
            return total;
        }

        int main()
        {
            int x = 4;
            int y = 7;
            return add(x, y);
        }
    )cpp";

    const auto disguised_a = R"cpp(
        int sum_up(int limit)
        {
            int total = 0;
            for (int i = 0; i < limit; ++i)
            {
                total = total + i;
            }
            return total;
        }
    )cpp";

    const auto disguised_b = R"cpp(
        // Same control flow with different names and noisy layout.
        int accumulate_values( int n )
        {
            int answer = 0; /* local accumulator */
            for ( int index = 0; index < n; ++index )
            {
                answer = answer + index;
            }
            return answer;
        }
    )cpp";

    const auto hello_world = R"cpp(
        #include <iostream>

        int main()
        {
            std::cout << "hello world\n";
            return 0;
        }
    )cpp";

    const auto sort_algo = R"cpp(
        void insertion_sort(int* data, int n)
        {
            for (int i = 1; i < n; ++i)
            {
                int key = data[i];
                int j = i - 1;
                while (j >= 0 && data[j] > key)
                {
                    data[j + 1] = data[j];
                    --j;
                }
                data[j + 1] = key;
            }
        }
    )cpp";

    std::cout << "EchoTrace fingerprint verification";
    if (k != 5 || window != 4)
    {
        std::cout << " (k=" << k << ", w=" << window << ")";
    }
    std::cout << '\n';
    std::cout << "----------------------------------\n";

    run_case("Exact match", exact_a, exact_b, "expected 100%", k, window);
    run_case("Disguised match", disguised_a, disguised_b, "expected > 80%", k, window);
    run_case("Different code", hello_world, sort_algo, "expected 0%", k, window);
}

void print_profile(std::string_view label,
                   const std::string& path,
                   const echotrace::DocumentProfile& profile)
{
    std::cout << label << ": " << path << '\n'
              << "  tokens:       " << profile.tokens << '\n'
              << "  k-grams:      " << profile.kgrams << '\n'
              << "  fingerprints: " << profile.fingerprint.size() << '\n';
}

void run_two_file_compare(const Cli& cli)
{
    const auto lhs = echotrace::io::read_file(cli.paths[0]);
    const auto rhs = echotrace::io::read_file(cli.paths[1]);

    const auto opts = norm_options(cli);
    const auto left = echotrace::parse_source(lhs, cli.k, cli.window, opts);
    const auto right = echotrace::parse_source(rhs, cli.k, cli.window, opts);
    const auto score = echotrace::sim::jaccard(left.fingerprint, right.fingerprint);

    std::cout << "EchoTrace file comparison\n";
    std::cout << "-------------------------\n";
    std::cout << "k-gram size:       " << cli.k << '\n';
    std::cout << "winnow window:     " << cli.window << "\n\n";

    print_profile("Left ", cli.paths[0], echotrace::to_profile(left));
    print_profile("Right", cli.paths[1], echotrace::to_profile(right));

    std::cout << "\nSimilarity: " << std::fixed << std::setprecision(2)
              << score << "%\n";
}

// ---- directory analysis --------------------------------------------------
void run_directory(const Cli& cli)
{
    const auto start = std::chrono::steady_clock::now();

    echotrace::analysis::AnalysisOptions opts;
    opts.k = cli.k;
    opts.window = cli.window;
    opts.threads = cli.threads;
    opts.threshold = cli.threshold;
    opts.top_n = cli.top_n;
    opts.normalization = norm_options(cli);

    auto result = echotrace::analysis::analyze_directory(cli.paths[0], opts);

    const auto end = std::chrono::steady_clock::now();
    const double duration_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    const auto format = echotrace::reporting::parse_format(cli.format);

    echotrace::reporting::ReportMeta meta;
    meta.root = cli.paths[0];
    meta.timestamp = now_iso();
    meta.duration_ms = duration_ms;
    meta.k = cli.k;
    meta.window = cli.window;
    meta.threads = cli.threads;
    meta.threshold = cli.threshold;
    meta.top_n = cli.top_n;

    const auto body = echotrace::reporting::render(result, meta, format);

    if (cli.output.empty())
    {
        std::cout << body;
    }
    else
    {
        std::ofstream out(cli.output, std::ios::binary);
        if (!out)
        {
            throw std::runtime_error("could not write report to '" + cli.output + "'");
        }
        out << body;
        out.close();
        if (!out)
        {
            throw std::runtime_error("error writing report to '" + cli.output + "'");
        }
        std::cerr << "Wrote " << result.summary.matches_above_threshold
                  << " matches to " << cli.output
                  << " (" << duration_ms << " ms, " << cli.format << ").\n";
    }

    for (const auto& w : result.warnings)
    {
        std::cerr << "warning: " << w << '\n';
    }
}

// Decide between two-file mode and directory mode based on the given paths.
bool is_directory_path(const std::string& p)
{
    std::error_code ec;
    return stdfs::is_directory(p, ec);
}

} // namespace

int main(int argc, char* argv[])
{
    try
    {
        const auto cli = parse_args(argc, argv);

        if (cli.help)
        {
            print_usage(std::cout, argv[0]);
            return 0;
        }
        if (cli.version)
        {
            std::cout << "EchoTrace " << echotrace::kVersion << '\n';
            return 0;
        }
        if (cli.self_test)
        {
            run_self_test(cli.k, cli.window);
            return 0;
        }

        if (cli.paths.empty())
        {
            throw std::invalid_argument("no input given (expected a directory or two files)");
        }

        if (cli.paths.size() == 1)
        {
            if (!is_directory_path(cli.paths[0]))
            {
                throw std::invalid_argument(
                    "'" + cli.paths[0] + "' is not a directory; "
                    "provide a directory or exactly two source files");
            }
            run_directory(cli);
            return 0;
        }

        if (cli.paths.size() == 2)
        {
            if (is_directory_path(cli.paths[0]) || is_directory_path(cli.paths[1]))
            {
                throw std::invalid_argument(
                    "two-file mode requires two files, not directories");
            }
            run_two_file_compare(cli);
            return 0;
        }

        throw std::invalid_argument(
            "expected a single directory or exactly two source files");
    }
    catch (const std::exception& ex)
    {
        std::cerr << "error: " << ex.what() << "\n\n";
        print_usage(std::cerr, argc > 0 ? argv[0] : "echotrace");
        return 1;
    }
}
