#ifndef ECHOTRACE_LANGUAGE_HPP
#define ECHOTRACE_LANGUAGE_HPP

#include <string>
#include <string_view>
#include <vector>

namespace echotrace {

enum class Language { Unknown, Cpp, Python, Java, JavaScript };

// Detect language from file extension
Language detect_language(const std::string& filename);

// Get display name
const char* language_name(Language lang);

// Get supported extensions for a language
const std::vector<std::string_view>& language_extensions(Language lang);

// Check if a file has any supported extension
bool has_any_supported_extension(const std::string& filename);

} // namespace echotrace

#endif // ECHOTRACE_LANGUAGE_HPP
