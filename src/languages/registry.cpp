// src/languages/registry.cpp
#include "echotrace/language_lexer.hpp"
#include <stdexcept>

namespace echotrace::lex {

extern const LanguageLexer& get_cpp_lexer();
extern const LanguageLexer& get_python_lexer();
extern const LanguageLexer& get_java_lexer();
extern const LanguageLexer& get_javascript_lexer();

const LanguageLexer& get_lexer(Language lang) {
    switch (lang) {
        case Language::Cpp: return get_cpp_lexer();
        case Language::Python: return get_python_lexer();
        case Language::Java: return get_java_lexer();
        case Language::JavaScript: return get_javascript_lexer();
        default: throw std::invalid_argument("Unsupported language lexer");
    }
}

} // namespace echotrace::lex
