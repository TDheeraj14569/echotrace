#include "test_framework.hpp"
#include "echotrace/language_lexer.hpp"
#include "echotrace/language.hpp"
#include <string_view>

TEST(CppLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::Cpp);
    auto tokens = lexer.tokenize_spans("int main() { // comment\n return 0; } /* block */");
    // Should skip comments
    ASSERT_EQ(tokens.size(), 7); // int, main, (, ), {, return, 0, }
    ASSERT_EQ(tokens[0].text, "int");
    ASSERT_EQ(tokens[1].text, "main");
    ASSERT_EQ(tokens[6].text, "}");
}

TEST(PythonLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::Python);
    auto tokens = lexer.tokenize_spans("def foo():\n  # comment\n  pass");
    ASSERT_EQ(tokens.size(), 5); // def, foo, (, ), pass
    ASSERT_EQ(tokens[0].text, "def");
    ASSERT_EQ(tokens[1].text, "foo");
    ASSERT_EQ(tokens[4].text, "pass");
}

TEST(JavaLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::Java);
    auto tokens = lexer.tokenize_spans("class Main { @Override void test() {} }");
    ASSERT_EQ(tokens.size(), 9); // class, Main, {, @Override, void, test, (, ), }
    ASSERT_EQ(tokens[3].text, "@"); // Or "@Override" depending on lexer implementation
}

TEST(JavaScriptLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::JavaScript);
    auto tokens = lexer.tokenize_spans("const x = () => { return a ?? b; };");
    // const, x, =, (, ), =>, {, return, a, ??, b, ;, }, ;
    ASSERT_EQ(tokens[0].text, "const");
    ASSERT_EQ(tokens[1].text, "x");
}
