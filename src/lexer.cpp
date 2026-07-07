#include "echotrace/lexer.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace echotrace
{

namespace lex
{

namespace
{

bool is_ident_start(unsigned char ch)
{
    return std::isalpha(ch) != 0 || ch == '_';
}

bool is_ident_body(unsigned char ch)
{
    return std::isalnum(ch) != 0 || ch == '_';
}

bool is_type(std::string_view s)
{
    static constexpr std::string_view types[] = {
        "bool",     "char",   "char8_t", "char16_t", "char32_t", "double",
        "float",    "int",    "long",    "short",    "signed",   "size_t",
        "uint8_t",  "uint16_t", "uint32_t", "uint64_t", "unsigned", "void",
        "wchar_t"
    };

    return std::find(std::begin(types), std::end(types), s) != std::end(types);
}

bool is_keyword(std::string_view s)
{
    static constexpr std::string_view keywords[] = {
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
    static constexpr std::string_view three_char_ops[] =
    {
        "<<=", ">>=", "..."
    };
    static constexpr std::string_view two_char_ops[] =
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
        if (std::isalnum(ch) != 0 || src[i] == '_' || src[i] == '.' || src[i] == '\'')
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

std::vector<Token> tokenize_spans(std::string_view src)
{
    std::vector<Token> tokens;
    tokens.reserve(src.size() / 3);

    auto emit = [&](std::string text, std::size_t off, std::size_t len)
    {
        tokens.push_back(Token{std::move(text), off, len});
    };

    for (std::size_t i = 0; i < src.size();)
    {
        const auto ch = static_cast<unsigned char>(src[i]);

        if (std::isspace(ch) != 0)
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
            const std::size_t start = i;
            skip_raw_string(src, i);
            emit("L", start, i - start);
            continue;
        }

        if (src[i] == '"' || src[i] == '\'')
        {
            const std::size_t start = i;
            skip_string_or_char(src, i, src[i]);
            emit("L", start, i - start);
            continue;
        }

        if (std::isdigit(ch) != 0 ||
            (src[i] == '.' && i + 1 < src.size() &&
             std::isdigit(static_cast<unsigned char>(src[i + 1])) != 0))
        {
            const std::size_t start = i;
            skip_number(src, i);
            emit("N", start, i - start);
            continue;
        }

        if (is_ident_start(ch))
        {
            const std::size_t start = i;
            ++i;
            while (i < src.size() && is_ident_body(static_cast<unsigned char>(src[i])) != 0)
            {
                ++i;
            }

            const auto text = src.substr(start, i - start);
            if (is_type(text))
            {
                emit("T", start, i - start);
            }
            else if (is_keyword(text))
            {
                emit(std::string(text.data(), text.size()), start, i - start);
            }
            else
            {
                emit("V", start, i - start);
            }
            continue;
        }

        const std::size_t start = i;
        std::string p = punct_token(src, i);
        emit(std::move(p), start, i - start);
    }

    return tokens;
}

std::vector<std::string> tokenize(std::string_view src)
{
    auto with_spans = tokenize_spans(src);
    std::vector<std::string> out;
    out.reserve(with_spans.size());
    for (auto& t : with_spans)
    {
        out.push_back(std::move(t.text));
    }
    return out;
}

} // namespace lex

} // namespace echotrace
