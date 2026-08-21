// src/languages/java_lexer.cpp
#include "echotrace/language_lexer.hpp"
#include <unordered_set>
#include <cctype>

namespace echotrace::lex {

class JavaLexer : public LanguageLexer {
public:
    std::vector<Token> tokenize_spans(std::string_view src) const override {
        std::vector<Token> tokens;
        size_t i = 0;
        size_t n = src.size();
        
        static const std::unordered_set<std::string_view> keywords = {
            "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class", 
            "const", "continue", "default", "do", "double", "else", "enum", "extends", "final", 
            "finally", "float", "for", "goto", "if", "implements", "import", "instanceof", "int", 
            "interface", "long", "native", "new", "package", "private", "protected", "public", 
            "return", "short", "static", "strictfp", "super", "switch", "synchronized", "this", 
            "throw", "throws", "transient", "try", "void", "volatile", "while", "true", "false", 
            "null", "var", "record", "sealed", "permits", "yield"
        };
        static const std::unordered_set<std::string_view> types = {
            "boolean", "byte", "char", "short", "int", "long", "float", "double", "void"
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
            
            if (c == '@') {
                size_t start = i;
                i++;
                while (i < n && is_id_cont(src[i])) i++;
                if (i > start + 1) {
                    tokens.push_back({"@", start, 1});
                    tokens.push_back({"V", start + 1, i - start - 1});
                    continue;
                }
                tokens.push_back({"@", start, 1});
                continue;
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
            
            if (c == '\'' || c == '"') {
                size_t start = i;
                char quote = c;
                
                bool is_text_block = (quote == '"' && i + 2 < n && src[i+1] == '"' && src[i+2] == '"');
                if (is_text_block) {
                    i += 3;
                    while (i < n) {
                        if (src[i] == '\\' && i + 1 < n) {
                            i += 2;
                        } else if (i + 2 < n && src[i] == '"' && src[i+1] == '"' && src[i+2] == '"') {
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
                if (two == "==" || two == "!=" || two == ">=" || two == "<=" || two == "&&" || two == "||" || two == "++" || two == "--" || two == "+=" || two == "-=" || two == "*=" || two == "/=" || two == "%=" || two == "&=" || two == "|=" || two == "^=" || two == "<<" || two == ">>" || two == "::" || two == "->") {
                    if (i + 2 < n) {
                        std::string_view three = src.substr(i, 3);
                        if (three == "<<=" || three == ">>=" || three == ">>>") {
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
    
    Language language() const override { return Language::Java; }
};

const LanguageLexer& get_java_lexer() {
    static JavaLexer instance;
    return instance;
}

} // namespace echotrace::lex
