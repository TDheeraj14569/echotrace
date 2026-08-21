#include "test_framework.hpp"
#include "echotrace/lexer.hpp"
#include <string>

TEST(LexerIdentifiersNormalized) {
    auto tokens = echotrace::lex::tokenize("myVar anotherVar");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(2));
    ASSERT_EQ(tokens[0], "V");
    ASSERT_EQ(tokens[1], "V");
}

TEST(LexerTypesNormalized) {
    auto tokens = echotrace::lex::tokenize("int double void");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(3));
    ASSERT_EQ(tokens[0], "T");
    ASSERT_EQ(tokens[1], "T");
    ASSERT_EQ(tokens[2], "T");
}

TEST(LexerNumbersNormalized) {
    auto tokens = echotrace::lex::tokenize("42 3.14 0xFF");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(3));
    ASSERT_EQ(tokens[0], "N");
    ASSERT_EQ(tokens[1], "N");
    ASSERT_EQ(tokens[2], "N");
}

TEST(LexerStringsNormalized) {
    auto tokens = echotrace::lex::tokenize("\"hello\" 'c'");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(2));
    ASSERT_EQ(tokens[0], "L");
    ASSERT_EQ(tokens[1], "L");
}

TEST(LexerKeywordsPreserved) {
    auto tokens = echotrace::lex::tokenize("if (x) return y;");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(7));
    ASSERT_EQ(tokens[0], "if");
    ASSERT_EQ(tokens[1], "(");
    ASSERT_EQ(tokens[2], "V");
    ASSERT_EQ(tokens[3], ")");
    ASSERT_EQ(tokens[4], "return");
    ASSERT_EQ(tokens[5], "V");
    ASSERT_EQ(tokens[6], ";");
}

TEST(LexerOperators) {
    auto tokens = echotrace::lex::tokenize("+ += <<=");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(3));
    ASSERT_EQ(tokens[0], "+");
    ASSERT_EQ(tokens[1], "+=");
    ASSERT_EQ(tokens[2], "<<=");
}

TEST(LexerLineComment) {
    auto tokens = echotrace::lex::tokenize("a // comment\nb");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(2));
    ASSERT_EQ(tokens[0], "V");
    ASSERT_EQ(tokens[1], "V");
}

TEST(LexerBlockComment) {
    auto tokens = echotrace::lex::tokenize("a /* block */ b");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(2));
    ASSERT_EQ(tokens[0], "V");
    ASSERT_EQ(tokens[1], "V");
}

TEST(LexerWhitespace) {
    auto tokens = echotrace::lex::tokenize("  a   b\t\tc  ");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(3));
}

TEST(LexerEmpty) {
    auto tokens = echotrace::lex::tokenize("");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(0));
}

TEST(LexerRawString) {
    auto tokens = echotrace::lex::tokenize("auto s = R\"(raw \" string)\";");
    ASSERT_EQ(tokens.size(), static_cast<size_t>(5));
    ASSERT_EQ(tokens[0], "auto");
    ASSERT_EQ(tokens[1], "V");
    ASSERT_EQ(tokens[2], "=");
    ASSERT_EQ(tokens[3], "L");
    ASSERT_EQ(tokens[4], ";");
}

TEST(LexerTokenOffsets) {
    auto spans = echotrace::lex::tokenize_spans("int a = 1;");
    ASSERT_EQ(spans.size(), static_cast<size_t>(5));
    ASSERT_EQ(spans[0].text, "T");  // int -> T
    ASSERT_EQ(spans[0].offset, static_cast<size_t>(0));
    ASSERT_EQ(spans[0].length, static_cast<size_t>(3));
    ASSERT_EQ(spans[1].text, "V");  // a -> V
    ASSERT_EQ(spans[1].offset, static_cast<size_t>(4));
    ASSERT_EQ(spans[1].length, static_cast<size_t>(1));
    ASSERT_EQ(spans[2].text, "=");
    ASSERT_EQ(spans[2].offset, static_cast<size_t>(6));
    ASSERT_EQ(spans[3].text, "N");  // 1 -> N
    ASSERT_EQ(spans[3].offset, static_cast<size_t>(8));
    ASSERT_EQ(spans[4].text, ";");
    ASSERT_EQ(spans[4].offset, static_cast<size_t>(9));
}

TEST(LexerTokenLengths) {
    auto spans = echotrace::lex::tokenize_spans("\"hello world\"");
    ASSERT_EQ(spans.size(), static_cast<size_t>(1));
    ASSERT_EQ(spans[0].text, "L");
    ASSERT_EQ(spans[0].offset, static_cast<size_t>(0));
    ASSERT_EQ(spans[0].length, static_cast<size_t>(13));
}
