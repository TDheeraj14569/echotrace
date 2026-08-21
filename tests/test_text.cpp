#include "test_framework.hpp"
#include "echotrace/text.hpp"
#include <string>
#include <stdexcept>

TEST(TextTrim) {
    ASSERT_EQ(echotrace::text::trim("  hello  "), "hello");
    ASSERT_EQ(echotrace::text::trim("world"), "world");
    ASSERT_EQ(echotrace::text::trim("  "), "");
    ASSERT_EQ(echotrace::text::trim(""), "");
    ASSERT_EQ(echotrace::text::trim("\t\n\r "), "");
}

TEST(TextToLower) {
    ASSERT_EQ(echotrace::text::to_lower("HeLLo"), "hello");
    ASSERT_EQ(echotrace::text::to_lower("123ABC"), "123abc");
    ASSERT_EQ(echotrace::text::to_lower(""), "");
}

TEST(TextEndsWithAny) {
    ASSERT_TRUE(echotrace::text::ends_with_any("test.cpp", {".cpp", ".hpp"}));
    ASSERT_TRUE(echotrace::text::ends_with_any("header.hpp", {".cpp", ".hpp"}));
    ASSERT_FALSE(echotrace::text::ends_with_any("file.txt", {".cpp", ".hpp"}));
    ASSERT_FALSE(echotrace::text::ends_with_any("", {".cpp"}));
}

TEST(TextHtmlEscape) {
    ASSERT_EQ(echotrace::text::html_escape("<b>&\"'</b>"),
              "&lt;b&gt;&amp;&quot;&#39;&lt;/b&gt;");
    ASSERT_EQ(echotrace::text::html_escape("hello"), "hello");
    ASSERT_EQ(echotrace::text::html_escape(""), "");
}

TEST(TextCsvEscape) {
    ASSERT_EQ(echotrace::text::csv_escape("hello, world"), "\"hello, world\"");
    ASSERT_EQ(echotrace::text::csv_escape("she said \"hi\""), "\"she said \"\"hi\"\"\"");
    ASSERT_EQ(echotrace::text::csv_escape("normal"), "normal");
    ASSERT_EQ(echotrace::text::csv_escape("line\nbreak"), "\"line\nbreak\"");
}

TEST(TextJsonEscape) {
    ASSERT_EQ(echotrace::text::json_escape("hello"), "hello");
    ASSERT_EQ(echotrace::text::json_escape("line\n"), "line\\n");
    ASSERT_EQ(echotrace::text::json_escape("tab\there"), "tab\\there");
    ASSERT_EQ(echotrace::text::json_escape("quote\"back\\"), "quote\\\"back\\\\");
}

TEST(IoReadFileThrows) {
    ASSERT_THROW(echotrace::io::read_file("nonexistent_file_12345.txt"), std::runtime_error);
}
