#include <algorithm>
#include <cctype>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#if __has_include(<string_view>)
#include <string_view>
#elif __has_include(<experimental/string_view>)
#include <experimental/string_view>
namespace std 
{
using string_view = experimental::string_view;
}
#else
#error "Moss-Lite requires std::string_view or std::experimental::string_view"
#endif

namespace moss_lite 
{

namespace lex 
{

namespace 
{

bool is_ident_start(unsigned char ch) 
{
    return std::isalpha(ch) || ch == '_';
}

bool is_ident_body(unsigned char ch) 
{
    return std::isalnum(ch) || ch == '_';
}

bool is_type(std::string_view s) 
{
    static const std::string_view types[] = {
        "bool",     "char",   "char8_t", "char16_t", "char32_t", "double",
        "float",    "int",    "long",    "short",    "signed",   "size_t",
        "uint8_t",  "uint16_t", "uint32_t", "uint64_t", "unsigned", "void",
        "wchar_t"
    };

    return std::find(std::begin(types), std::end(types), s) != std::end(types);
}

bool is_keyword(std::string_view s) 
{
    static const std::string_view keywords[] = {
        "alignas",    "alignof",  "and",        "and_eq",    "asm",
        "auto",       "bitand",   "bitor",      "break",     "case",
        "catch",      "class",    "compl",      "concept",   "const",
        "consteval",  "constexpr", "constinit", "const_cast", "continue",
        "co_await",   "co_return", "co_yield",  "decltype",  "default",
        "delete",     "do",       "dynamic_cast", "else",    "enum",
        "explicit",   "export",   "extern",     "false",     "for",
        "friend",     "goto",     "if",         "inline",    "mutable",
        "namespace",  "new",      "noexcept",   "not",       "not_eq",
        "nullptr",    "operator", "or",         "or_eq",     "private",
        "protected",  "public",   "register",   "reinterpret_cast",
        "requires",   "return",   "sizeof",     "static",    "static_assert",
        "static_cast", "struct",  "switch",     "template",  "this",
        "thread_local", "throw",  "true",       "try",       "typedef",
        "typeid",     "typename", "union",      "using",     "virtual",
        "volatile",   "while",    "xor",        "xor_eq"
    };

    return std::find(std::begin(keywords), std::end(keywords), s) != std::end(keywords);
}

bool starts_with(std::string_view s, std::size_t pos, std::string_view needle) 
{
    return pos + needle.size() <= s.size() && s.substr(pos, needle.size()) == needle;
}

std::string punct_token(std::string_view src, std::size_t& i) 
{
    static const std::string_view three_char_ops[] = 
    {
        "<<=", ">>=", "..."
    };
    static const std::string_view two_char_ops[] = 
    {
        "::", "->", "++", "--", "<<", ">>", "<=", ">=", "==", "!=",
        "&&", "||", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "##"
    };

    for (auto op : three_char_ops) 
    {
        if (starts_with(src, i, op)) 
        {
            i += op.size();
            return std::string(op.data(), op.size());
        }
    }

    for (auto op : two_char_ops) 
    {
        if (starts_with(src, i, op)) 
        {
            i += op.size();
            return std::string(op.data(), op.size());
        }
    }

    return std::string(1, src[i++]);
}

void skip_string_or_char(std::string_view src, std::size_t& i, char quote) 
{
    ++i;
    while (i < src.size()) 
    {
        if (src[i] == '\\') 
        {
            i = std::min(i + 2, src.size());
            continue;
        }
        if (src[i++] == quote) 
        {
            return;
        }
    }
}

void skip_raw_string(std::string_view src, std::size_t& i) 
{
    const auto open = src.find('(', i);
    if (open == std::string_view::npos) 
    {
        i = src.size();
        return;
    }

    const auto delimiter = src.substr(i + 2, open - (i + 2));
    const std::string close = ")" + std::string(delimiter.data(), delimiter.size()) + "\"";
    const auto end = src.find(close, open + 1);
    i = (end == std::string_view::npos) ? src.size() : end + close.size();
}

void skip_number(std::string_view src, std::size_t& i) 
{
    bool prev_exp = false;
    while (i < src.size()) 
    {
        const auto ch = static_cast<unsigned char>(src[i]);
        if (std::isalnum(ch) || src[i] == '_' || src[i] == '.' || src[i] == '\'') 
        {
            prev_exp = src[i] == 'e' || src[i] == 'E' || src[i] == 'p' || src[i] == 'P';
            ++i;
            continue;
        }
        if ((src[i] == '+' || src[i] == '-') && prev_exp) 
        {
            prev_exp = false;
            ++i;
            continue;
        }
        break;
    }
}

} // namespace

std::vector<std::string> tokenize(std::string_view src) 
{
    std::vector<std::string> tokens;
    tokens.reserve(src.size() / 3);

    for (std::size_t i = 0; i < src.size();) 
    {
        const auto ch = static_cast<unsigned char>(src[i]);

        if (std::isspace(ch)) 
        {
            ++i;
            continue;
        }

        if (starts_with(src, i, "//")) 
        {
            i = src.find('\n', i + 2);
            if (i == std::string_view::npos) 
            {
                break;
            }
            continue;
        }

        if (starts_with(src, i, "/*")) 
        {
            const auto end = src.find("*/", i + 2);
            i = (end == std::string_view::npos) ? src.size() : end + 2;
            continue;
        }

        if (starts_with(src, i, "R\"")) 
        {
            skip_raw_string(src, i);
            tokens.emplace_back("L");
            continue;
        }

        if (src[i] == '"' || src[i] == '\'') 
        {
            skip_string_or_char(src, i, src[i]);
            tokens.emplace_back("L");
            continue;
        }

        if (std::isdigit(ch) || (src[i] == '.' && i + 1 < src.size() &&
                                 std::isdigit(static_cast<unsigned char>(src[i + 1])))) 
                                 {
            skip_number(src, i);
            tokens.emplace_back("N");
            continue;
        }

        if (is_ident_start(ch)) 
        {
            const auto start = i++;
            while (i < src.size() && is_ident_body(static_cast<unsigned char>(src[i]))) 
            {
                ++i;
            }

            const auto text = src.substr(start, i - start);
            if (is_type(text)) {
                tokens.emplace_back("T");
            } else if (is_keyword(text)) 
            {
                tokens.emplace_back(text.data(), text.size());
            } else 
            {
                tokens.emplace_back("V");
            }
            continue;
        }

        tokens.emplace_back(punct_token(src, i));
    }

    return tokens;
}

} // namespace lex

namespace kgram {

std::vector<std::string> make(const std::vector<std::string>& tokens, std::size_t k = 5) 
{
    if (k == 0) 
    {
        throw std::invalid_argument("k-gram size must be positive");
    }
    if (tokens.size() < k) 
    {
        return {};
    }

    std::vector<std::string> grams;
    grams.reserve(tokens.size() - k + 1);

    for (std::size_t i = 0; i + k <= tokens.size(); ++i) 
    {
        std::string gram;
        for (std::size_t j = 0; j < k; ++j) 
        {
            if (j != 0) {
                gram.push_back(' ');
            }
            gram += tokens[i + j];
        }
        grams.push_back(std::move(gram));
    }

    return grams;
}

} // namespace kgram

namespace hash 
{

namespace 
{

constexpr std::uint64_t base = 256;
constexpr std::uint64_t mod = 1'000'000'007;

std::uint64_t pow_mod(std::uint64_t x, std::size_t exp) 
{
    std::uint64_t out = 1;
    while (exp-- > 0) 
    {
        out = (out * x) % mod;
    }
    return out;
}

std::uint64_t symbol(std::string_view token) 
{
    std::uint64_t h = 1469598103934665603ull;
    for (unsigned char ch : token) 
    {
        h ^= ch;
        h *= 1099511628211ull;
    }
    return (h % (mod - 1)) + 1;
}

} // namespace

std::vector<std::uint64_t> rolling_token_hashes(const std::vector<std::string>& tokens,
                                                std::size_t k = 5) 
                                                {
    if (k == 0) 
    {
        throw std::invalid_argument("hash window size must be positive");
    }
    if (tokens.size() < k) 
    {
        return {};
    }

    std::vector<std::uint64_t> syms;
    syms.reserve(tokens.size());
    for (const auto& token : tokens) 
    {
        syms.push_back(symbol(token));
    }

    std::vector<std::uint64_t> hashes;
    hashes.reserve(tokens.size() - k + 1);

    std::uint64_t h = 0;
    for (std::size_t i = 0; i < k; ++i) 
    {
        h = (h * base + syms[i]) % mod;
    }
    hashes.push_back(h);

    // Token symbols, rather than bytes, keep aliases such as "V" and "T" from
    // being weighted by the spelling length of adjacent operators or keywords.
    const auto lead_factor = pow_mod(base, k - 1);
    for (std::size_t i = k; i < syms.size(); ++i) 
    {
        const auto lead = (syms[i - k] * lead_factor) % mod;
        h = (h + mod - lead) % mod;
        h = (h * base + syms[i]) % mod;
        hashes.push_back(h);
    }

    return hashes;
}

} // namespace hash

namespace fingerprint 
{

std::unordered_set<std::uint64_t> winnow(const std::vector<std::uint64_t>& hashes,
                                         std::size_t window = 4) 
                                         {
    if (window == 0) 
    {
        throw std::invalid_argument("winnowing window must be positive");
    }
    if (hashes.empty()) 
    {
        return {};
    }

    std::deque<std::pair<std::size_t, std::uint64_t>> mins;
    std::unordered_set<std::uint64_t> fp;
    fp.reserve(hashes.size() / window + 1);

    std::size_t last_pick = hashes.size();

    for (std::size_t i = 0; i < hashes.size(); ++i) 
    {
        while (!mins.empty() && mins.front().first + window <= i) 
        {
            mins.pop_front();
        }

        while (!mins.empty() && mins.back().second >= hashes[i]) 
        {
            mins.pop_back();
        }
        mins.emplace_back(i, hashes[i]);

        if (i + 1 >= window && mins.front().first != last_pick) 
        {
            fp.insert(mins.front().second);
            last_pick = mins.front().first;
        }
    }

    if (hashes.size() < window) 
    {
        fp.insert(mins.front().second);
    }

    return fp;
}

std::unordered_set<std::uint64_t> from_source(std::string_view src,
                                              std::size_t k = 5,
                                              std::size_t window = 4) 
                                              {
    auto tokens = lex::tokenize(src);
    auto hashes = hash::rolling_token_hashes(tokens, k);
    return winnow(hashes, window);
}

} // namespace fingerprint

namespace sim 
{

double jaccard(const std::unordered_set<std::uint64_t>& a,
               const std::unordered_set<std::uint64_t>& b) 
               {
    if (a.empty() && b.empty()) 
    {
        return 100.0;
    }

    const auto& small = (a.size() < b.size()) ? a : b;
    const auto& large = (a.size() < b.size()) ? b : a;

    std::size_t intersection = 0;
    for (const auto h : small) 
    {
        intersection += large.find(h) != large.end() ? 1u : 0u;
    }

    const auto uni = a.size() + b.size() - intersection;
    return uni == 0 ? 100.0 : (100.0 * static_cast<double>(intersection)) / static_cast<double>(uni);
}

double compare(std::string_view lhs, std::string_view rhs,
               std::size_t k = 5,
               std::size_t window = 4) 
               {
    const auto a = fingerprint::from_source(lhs, k, window);
    const auto b = fingerprint::from_source(rhs, k, window);
    return jaccard(a, b);
}

} // namespace sim

namespace io {

std::string read_file(const std::string& path) 
{
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) 
    {
        throw std::runtime_error("could not open '" + path + "'");
    }

    const auto end = in.tellg();
    if (end < 0) 
    {
        throw std::runtime_error("could not measure '" + path + "'");
    }

    std::string out(static_cast<std::size_t>(end), '\0');
    in.seekg(0, std::ios::beg);

    if (!out.empty()) 
    {
        in.read(&out[0], static_cast<std::streamsize>(out.size()));
        if (!in) 
        {
            throw std::runtime_error("could not read '" + path + "'");
        }
    }

    return out;
}

} // namespace io

namespace report 
{

struct DocumentProfile 
{
    std::size_t tokens = 0;
    std::size_t kgrams = 0;
    std::unordered_set<std::uint64_t> fingerprint;
};

struct Comparison 
{
    DocumentProfile lhs;
    DocumentProfile rhs;
    double similarity = 0.0;
};

DocumentProfile profile(std::string_view src, std::size_t k, std::size_t window) 
{
    auto tokens = lex::tokenize(src);
    auto hashes = hash::rolling_token_hashes(tokens, k);
    auto fp = fingerprint::winnow(hashes, window);

    return DocumentProfile{
        tokens.size(),
        hashes.size(),
        std::move(fp)
    };
}

Comparison compare_sources(std::string_view lhs,
                           std::string_view rhs,
                           std::size_t k,
                           std::size_t window) 
                           {
    auto left = profile(lhs, k, window);
    auto right = profile(rhs, k, window);
    const auto score = sim::jaccard(left.fingerprint, right.fingerprint);

    return Comparison
    {
        std::move(left),
        std::move(right),
        score
    };
}

} // namespace report

} // namespace moss_lite

namespace 
{

struct Cli 
{
    bool help = false;
    bool self_test = false;
    std::size_t k = 5;
    std::size_t window = 4;
    std::vector<std::string> paths;
};

bool arg_starts_with(std::string_view arg, std::string_view prefix) 
{
    return arg.size() >= prefix.size() && arg.substr(0, prefix.size()) == prefix;
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
    } catch (const std::exception&) 
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

Cli parse_args(int argc, char* argv[]) 
{
    Cli cli;

    for (int i = 1; i < argc; ++i) {
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
        } else if (arg == "--self-test") 
        {
            cli.self_test = true;
        } else if (arg == "--k" || arg == "-k") 
        {
            cli.k = parse_size(take_next(arg), arg);
        } else if (arg_starts_with(arg, "--k=")) 
        {
            cli.k = parse_size(arg.substr(4), "--k");
        } else if (arg == "--w" || arg == "-w") 
        {
            cli.window = parse_size(take_next(arg), arg);
        } else if (arg_starts_with(arg, "--w=")) 
        {
            cli.window = parse_size(arg.substr(4), "--w");
        } else if (!arg.empty() && arg[0] == '-') 
        {
            throw std::invalid_argument("unknown option '" + str(arg) + "'");
        } else 
        {
            cli.paths.emplace_back(arg.data(), arg.size());
        }
    }

    if (!cli.help && cli.self_test && !cli.paths.empty()) 
    {
        throw std::invalid_argument("--self-test does not accept file paths");
    }
    if (!cli.help && !cli.self_test && cli.paths.size() != 2) 
    {
        throw std::invalid_argument("expected exactly two source file paths");
    }

    return cli;
}

void print_usage(std::ostream& out, std::string_view exe) 
{
    out << "Usage:\n"
        << "  " << exe << " <left.cpp> <right.cpp> [--k N] [--w N]\n"
        << "  " << exe << " --self-test\n"
        << "  " << exe << " --help\n\n"
        << "Options:\n"
        << "  -k, --k N    token k-gram size (default 5)\n"
        << "  -w, --w N    winnowing window size (default 4)\n";
}

void run_case(std::string_view name,
              std::string_view lhs,
              std::string_view rhs,
              std::string_view expectation,
              std::size_t k,
              std::size_t window) 
              {
    const auto score = moss_lite::sim::compare(lhs, rhs, k, window);

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

    std::cout << "Moss-Lite fingerprint verification";
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
                   const moss_lite::report::DocumentProfile& profile) 
                   {
    std::cout << label << ": " << path << '\n'
              << "  tokens:      " << profile.tokens << '\n'
              << "  k-grams:     " << profile.kgrams << '\n'
              << "  fingerprints:" << ' ' << profile.fingerprint.size() << '\n';
}

void run_file_compare(const Cli& cli) 
{
    const auto lhs = moss_lite::io::read_file(cli.paths[0]);
    const auto rhs = moss_lite::io::read_file(cli.paths[1]);
    const auto result = moss_lite::report::compare_sources(lhs, rhs, cli.k, cli.window);

    std::cout << "Moss-Lite file comparison\n";
    std::cout << "-------------------------\n";
    std::cout << "k-gram size:       " << cli.k << '\n';
    std::cout << "winnow window:     " << cli.window << "\n\n";

    print_profile("Left", cli.paths[0], result.lhs);
    print_profile("Right", cli.paths[1], result.rhs);

    std::cout << "\nSimilarity: " << std::fixed << std::setprecision(2)
              << result.similarity << "%\n";
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

        if (cli.self_test) 
        {
            run_self_test(cli.k, cli.window);
            return 0;
        }

        run_file_compare(cli);
        return 0;
    } 
    catch (const std::exception& ex) 
    {
        std::cerr << "error: " << ex.what() << "\n\n";
        print_usage(std::cerr, argc > 0 ? argv[0] : "moss_lite");
        return 1;
    }
}
