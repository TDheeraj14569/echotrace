// src/languages/python_lexer.cpp
#include "echotrace/language_lexer.hpp"
#include <unordered_set>
#include <cctype>

namespace echotrace::lex {

class PythonLexer : public LanguageLexer {
public:
    std::vector<Token> tokenize_spans(std::string_view src) const override {
        std::vector<Token> tokens;
        size_t i = 0;
        size_t n = src.size();
        
        static const std::unordered_set<std::string_view> keywords = {
            "def", "class", "return", "if", "elif", "else", "for", "while", "import", "from", "as", 
            "with", "try", "except", "finally", "raise", "yield", "lambda", "pass", "break", 
            "continue", "and", "or", "not", "is", "in", "True", "False", "global", "nonlocal", 
            "del", "assert", "async", "await"
        };
        static const std::unordered_set<std::string_view> types = {
            "int", "float", "str", "bool", "list", "dict", "tuple", "set", "bytes", "None"
        };

        auto is_id_start = [](char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; };
        auto is_id_cont = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
        
        while (i < n) {
            char c = src[i];
            
            if (std::isspace(static_cast<unsigned char>(c))) {
                i++;
                continue;
            }
            
            if (c == '#') {
                while (i < n && src[i] != '\n') i++;
                continue;
            }
            
            if (is_id_start(c)) {
                size_t start = i;
                bool is_string_prefix = false;
                if (i + 1 < n && (src[i+1] == '\'' || src[i+1] == '"')) {
                    if (c == 'r' || c == 'R' || c == 'f' || c == 'F' || c == 'b' || c == 'B' || c == 'u' || c == 'U') {
                        is_string_prefix = true;
                    }
                }
                if (i + 2 < n && (src[i+2] == '\'' || src[i+2] == '"')) {
                    char c2 = src[i+1];
                    if ((c == 'f' || c == 'F' || c == 'b' || c == 'B' || c == 'r' || c == 'R') &&
                        (c2 == 'f' || c2 == 'F' || c2 == 'b' || c2 == 'B' || c2 == 'r' || c2 == 'R')) {
                        is_string_prefix = true;
                    }
                }
                
                if (is_string_prefix) {
                    while (i < n && src[i] != '\'' && src[i] != '"') i++;
                    if (i < n) c = src[i]; else continue;
                } else {
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
            }
            
            if (c == '\'' || c == '"') {
                size_t start = i;
                char quote = c;
                bool is_triple = (i + 2 < n && src[i+1] == quote && src[i+2] == quote);
                if (is_triple) {
                    i += 3;
                    while (i < n) {
                        if (src[i] == '\\' && i + 1 < n) {
                            i += 2;
                        } else if (i + 2 < n && src[i] == quote && src[i+1] == quote && src[i+2] == quote) {
                            i += 3;
                            break;
                        } else {
                            i++;
                        }
                    }
                } else {
                    i++;
                    while (i < n && src[i] != quote && src[i] != '\n') {
                        if (src[i] == '\\' && i + 1 < n) {
                            i += 2;
                        } else {
                            i++;
                        }
                    }
                    if (i < n && src[i] == quote) i++;
                }
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
                if (two == "//" || two == "**" || two == ":=" || two == "->" || two == "==" || two == "!=" || two == ">=" || two == "<=" || two == "+=" || two == "-=" || two == "*=" || two == "/=" || two == "%=" || two == "&=" || two == "|=" || two == "^=" || two == "<<") {
                    if (i + 2 < n) {
                        std::string_view three = src.substr(i, 3);
                        if (three == "//=" || three == "**=" || three == "<<=" || three == ">>=") {
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
    
    Language language() const override { return Language::Python; }
};

const LanguageLexer& get_python_lexer() {
    static PythonLexer instance;
    return instance;
}

} // namespace echotrace::lex
