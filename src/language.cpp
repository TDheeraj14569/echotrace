#include "echotrace/language.hpp"
#include "echotrace/text.hpp"
#include <filesystem>
#include <algorithm>

namespace echotrace {

static const std::vector<std::string_view> ext_cpp = {".c", ".h", ".cpp", ".cc", ".cxx", ".hpp"};
static const std::vector<std::string_view> ext_python = {".py", ".pyw"};
static const std::vector<std::string_view> ext_java = {".java"};
static const std::vector<std::string_view> ext_javascript = {".js", ".jsx", ".ts", ".tsx"};
static const std::vector<std::string_view> ext_empty = {};

Language detect_language(const std::string& filename) {
    std::filesystem::path p(filename);
    std::string ext = text::to_lower(p.extension().string());

    auto contains = [&ext](const std::vector<std::string_view>& exts) {
        return std::find(exts.begin(), exts.end(), ext) != exts.end();
    };

    if (contains(ext_cpp)) return Language::Cpp;
    if (contains(ext_python)) return Language::Python;
    if (contains(ext_java)) return Language::Java;
    if (contains(ext_javascript)) return Language::JavaScript;
    
    return Language::Unknown;
}

const char* language_name(Language lang) {
    switch (lang) {
        case Language::Cpp: return "C++";
        case Language::Python: return "Python";
        case Language::Java: return "Java";
        case Language::JavaScript: return "JavaScript/TypeScript";
        default: return "Unknown";
    }
}

const std::vector<std::string_view>& language_extensions(Language lang) {
    switch (lang) {
        case Language::Cpp: return ext_cpp;
        case Language::Python: return ext_python;
        case Language::Java: return ext_java;
        case Language::JavaScript: return ext_javascript;
        default: return ext_empty;
    }
}

bool has_any_supported_extension(const std::string& filename) {
    return detect_language(filename) != Language::Unknown;
}

} // namespace echotrace
