// src/languages/javascript_lexer.cpp
#include "echotrace/language_lexer.hpp"
#include <unordered_set>
#include <cctype>

namespace echotrace::lex {

class JavaScriptLexer : public LanguageLexer {
public:
    std::vector<Token> tokenize_spans(std::string_view src) const override {
        std::vector<Token> tokens;
        size_t i = 0;
        size_t n = src.size();
        
        static const std::unordered_set<std::string_view> keywords = {
            "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", 
            "do", "else", "enum", "export", "extends", "false", "finally", "for", "function", "if", 
            "import", "in", "instanceof", "let", "new", "null", "of", "return", "super", "switch", 
            "this", "throw", "true", "try", "typeof", "undefined", "var", "void", "while", "with", 
            "yield", "async", "await", "implements", "interface", "package", "private", "protected", 
            "public", "static", "type", "as", "from", "namespace", "declare", "readonly", "abstract", "override"
        };
        static const std::unordered_set<std::string_view> types = {
            "number", "string", "boolean", "any", "void", "never", "unknown", "object", "symbol", "bigint"
        };

        auto is_id_start = [](char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '$'; };
        auto is_id_cont = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$'; };
        
        while (i < n) {
            char c = src[i];
            
            if (std::isspace(static_cast<unsigned char>(c))) {
                i++;
                continue;
            }
            
            if (c == '/' && i + 1 < n) {
                if (src[i+1] == '/') {
                    i += 2;
                    while (i < n && src[i] != '\n') i++;
                    continue;
                } else if (src[i+1] == '*') {
                    i += 2;
                    while (i < n) {
                        if (src[i] == '*' && i + 1 < n && src[i+1] == '/') {
                            i += 2;
                            break;
                        }
                        i++;
                    }
                    continue;
                }
            }
            
            if (is_id_start(c)) {
                size_t start = i;
                while (i < n && is_id_cont(src[i])) i++;
                std::string_view id = src.substr(start, i - start);
                if (types.count(id)) {
                    tokens.push_back({"T", start, i - start});
                } else if (keywords.count(id)) {
                    tokens.push_back({std::string(id), start, i - start});
                } else {
                    tokens.push_back({"V", start, i - start});
                }
                continue;
            }
            
            if (c == '\'' || c == '"' || c == '`') {
                size_t start = i;
                char quote = c;
                i++;
                while (i < n && src[i] != quote) {
                    if (src[i] == '\\' && i + 1 < n) {
                        i += 2;
                    } else {
                        i++;
                    }
                }
                if (i < n && src[i] == quote) i++;
                tokens.push_back({"L", start, i - start});
                continue;
            }
            
            if (std::isdigit(static_cast<unsigned char>(c)) || (c == '.' && i + 1 < n && std::isdigit(static_cast<unsigned char>(src[i+1])))) {
                size_t start = i;
                while (i < n && (std::isalnum(static_cast<unsigned char>(src[i])) || src[i] == '.' || src[i] == '_')) {
                    i++;
                }
                tokens.push_back({"N", start, i - start});
                continue;
            }
            
            size_t start = i;
            if (i + 1 < n) {
                std::string_view two = src.substr(i, 2);
                if (two == "==" || two == "!=" || two == ">=" || two == "<=" || two == "&&" || two == "||" || two == "++" || two == "--" || two == "+=" || two == "-=" || two == "*=" || two == "/=" || two == "%=" || two == "&=" || two == "|=" || two == "^=" || two == "<<" || two == ">>" || two == "=>" || two == "?." || two == "??" || two == "**") {
                    if (i + 2 < n) {
                        std::string_view three = src.substr(i, 3);
                        if (three == "===" || three == "!==" || three == "<<=" || three == ">>=" || three == ">>>" || three == "?\\?=" || three == "**=" || three == "...") {
                            if (i + 3 < n && src.substr(i, 4) == ">>>=") {
                                tokens.push_back({std::string(src.substr(i, 4)), start, 4});
                                i += 4;
                                continue;
                            }
                            tokens.push_back({std::string(three), start, 3});
                            i += 3;
                            continue;
                        }
                    }
                    tokens.push_back({std::string(two), start, 2});
                    i += 2;
                    continue;
                }
            }
            
            tokens.push_back({std::string(1, c), start, 1});
            i++;
        }
        
        return tokens;
    }
    
    Language language() const override { return Language::JavaScript; }
};

const LanguageLexer& get_javascript_lexer() {
    static JavaScriptLexer instance;
    return instance;
}

} // namespace echotrace::lex
