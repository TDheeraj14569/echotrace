#ifndef ECHOTRACE_LEXER_HPP
#define ECHOTRACE_LEXER_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace echotrace
{

/// Lexical analysis and token-level normalisation.
///
/// The tokenizer is the engine's first normalisation stage: it discards
/// whitespace and comments, collapses every user identifier to the symbol
/// ``"V"``, every type to ``"T"``, every numeric literal to ``"N"`` and every
/// string/character literal to ``"L"``. Keywords and operators are preserved
/// verbatim because they carry structural meaning that plagiarism detection
/// relies upon. This means two programs that differ only in identifier
/// spelling, literal values and layout produce identical token streams.
namespace lex
{

/// A token plus its character span in the original source. [offset] is the
/// byte offset of the first character; [length] is the span length. The span
/// always covers the ORIGINAL source text (e.g. an entire string literal or
/// identifier), enabling matched-fragment highlighting.
struct Token
{
    std::string text;
    std::size_t offset = 0;
    std::size_t length = 0;
};

/// Tokenise [src] into a sequence of normalised token symbols with positions.
///
/// Complexity: O(n) in the source length, single pass, no back-tracking.
std::vector<Token> tokenize_spans(std::string_view src);

/// Tokenise [src] into a sequence of normalised token symbols (positions
/// discarded). Equivalent to projecting tokenize_spans().text.
std::vector<std::string> tokenize(std::string_view src);

} // namespace lex

} // namespace echotrace

#endif // ECHOTRACE_LEXER_HPP
