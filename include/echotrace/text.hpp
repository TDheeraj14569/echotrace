#ifndef ECHOTRACE_TEXT_HPP
#define ECHOTRACE_TEXT_HPP

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

namespace echotrace
{

/// Filesystem and encoding helpers shared by every layer of the engine.
/// Keeping these in one place avoids duplicated escaping/normalisation logic
/// across the CLI, the report writers and the (future) web backend.
namespace io
{

/// Read an entire file into a string.
///
/// Reads in binary mode so byte-for-byte fidelity is preserved for Unicode
/// and platform-specific line endings. Throws std::runtime_error with a
/// meaningful message if the file cannot be opened, measured or read.
std::string read_file(const std::string& path);

} // namespace io

/// Pure text utilities with no filesystem dependencies.
namespace text
{

/// Trim leading and trailing ASCII whitespace from [s].
std::string trim(std::string_view s);

/// Lower-case an ASCII string (used for extension matching, never locale-aware).
std::string to_lower(std::string_view s);

/// True if [s] ends with any of [suffixes] (case-insensitive ASCII).
bool ends_with_any(std::string_view s, std::initializer_list<std::string_view> suffixes);

/// Escape [s] for safe inclusion in an HTML text node / attribute.
std::string html_escape(std::string_view s);

/// Quote and escape [s] for RFC 4180 CSV output.
std::string csv_escape(std::string_view s);

/// Escape [s] for safe inclusion inside a JSON string literal (without quotes).
std::string json_escape(std::string_view s);

} // namespace text

} // namespace echotrace

#endif // ECHOTRACE_TEXT_HPP
