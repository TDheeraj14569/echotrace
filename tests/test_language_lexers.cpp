#include "test_framework.hpp"
#include "echotrace/language_lexer.hpp"
#include "echotrace/language.hpp"
#include <string_view>

TEST(CppLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::Cpp);
    auto tokens = lexer.tokenize_spans("int main() { // comment\n return 0; } /* block */");
    // T(int), V(main), (, ), {, return, N(0), ;, }
    ASSERT_EQ(tokens.size(), 9u);
    ASSERT_EQ(tokens[0].text, "T");
    ASSERT_EQ(tokens[1].text, "V");
    ASSERT_EQ(tokens[8].text, "}");
}

TEST(PythonLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::Python);
    auto tokens = lexer.tokenize_spans("def foo():\n  # comment\n  pass");
    ASSERT_EQ(tokens.size(), 6u); // def, V(foo), (, ), :, pass
    ASSERT_EQ(tokens[0].text, "def");
    ASSERT_EQ(tokens[4].text, ":");
    ASSERT_EQ(tokens[5].text, "pass");
}

TEST(JavaLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::Java);
    auto tokens = lexer.tokenize_spans("class Main { @Override void test() {} }");
    ASSERT_EQ(tokens.size(), 12u); // class, V(Main), {, @, V(Override), T(void), V(test), (, ), {, } }
    ASSERT_EQ(tokens[3].text, "@");
}

TEST(JavaScriptLexerBasic) {
    const auto& lexer = echotrace::lex::get_lexer(echotrace::Language::JavaScript);
    auto tokens = lexer.tokenize_spans("const x = () => { return a ?? b; };");
    // const, V(x), =, (, ), =>, {, return, V(a), ??, V(b), ;, }, ;
    ASSERT_EQ(tokens.size(), 14u);
    ASSERT_EQ(tokens[0].text, "const");
}
