#include "echotrace/text.hpp"

#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace echotrace
{

namespace io
{

// Preserved verbatim from the original moss_lite implementation: binary read
// using seek-to-end to size the buffer, with explicit diagnostics on failure.
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

namespace text
{

std::string trim(std::string_view s)
{
    std::size_t b = 0;
    std::size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\n' || s[b] == '\r'))
    {
        ++b;
    }
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\n' || s[e - 1] == '\r'))
    {
        --e;
    }
    return std::string(s.data() + b, e - b);
}

std::string to_lower(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        out.push_back(static_cast<char>(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c));
    }
    return out;
}

bool ends_with_any(std::string_view s, std::initializer_list<std::string_view> suffixes)
{
    for (auto suf : suffixes)
    {
        if (s.size() >= suf.size() && s.substr(s.size() - suf.size()) == suf)
        {
            return true;
        }
    }
    return false;
}

std::string html_escape(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
        case '&':  out += "&amp;"; break;
        case '<':  out += "&lt;"; break;
        case '>':  out += "&gt;"; break;
        case '"':  out += "&quot;"; break;
        case '\'': out += "&#39;"; break;
        default:   out.push_back(c); break;
        }
    }
    return out;
}

std::string csv_escape(std::string_view s)
{
    const bool needs_quote = s.find_first_of(",\"\n\r") != std::string_view::npos;
    std::string out;
    out.reserve(s.size() + 2);
    if (needs_quote)
    {
        out.push_back('"');
        for (char c : s)
        {
            if (c == '"')
            {
                out += "\"\"";
            }
            else
            {
                out.push_back(c);
            }
        }
        out.push_back('"');
    }
    else
    {
        out.append(s.data(), s.size());
    }
    return out;
}

std::string json_escape(std::string_view s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c)
        {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20)
            {
                char buf[7];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
                out += buf;
            }
            else
            {
                out.push_back(static_cast<char>(c));
            }
            break;
        }
    }
    return out;
}

} // namespace text

} // namespace echotrace
