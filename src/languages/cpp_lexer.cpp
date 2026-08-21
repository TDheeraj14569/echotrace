// src/languages/cpp_lexer.cpp
#include "echotrace/language_lexer.hpp"
#include "echotrace/lexer.hpp"

namespace echotrace::lex {

class CppLexer : public LanguageLexer {
public:
    std::vector<Token> tokenize_spans(std::string_view src) const override {
        return echotrace::lex::tokenize_spans(src);
    }
    
    Language language() const override {
        return Language::Cpp;
    }
};

const LanguageLexer& get_cpp_lexer() {
    static CppLexer instance;
    return instance;
}

} // namespace echotrace::lex
