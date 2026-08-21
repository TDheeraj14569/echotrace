#ifndef ECHOTRACE_LANGUAGE_LEXER_HPP
#define ECHOTRACE_LANGUAGE_LEXER_HPP

#include "echotrace/language.hpp"
#include "echotrace/lexer.hpp"
#include <string_view>
#include <vector>

namespace echotrace::lex {

struct LanguageLexer {
    virtual ~LanguageLexer() = default;
    virtual std::vector<Token> tokenize_spans(std::string_view src) const = 0;
    virtual Language language() const = 0;
};

// Factory: get a lexer for a language
const LanguageLexer& get_lexer(Language lang);

} // namespace echotrace::lex

#endif // ECHOTRACE_LANGUAGE_LEXER_HPP
