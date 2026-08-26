#include "test_framework.hpp"
#include "echotrace/fs_traversal.hpp"
#include <stdexcept>

TEST(FsTraversalSupportedExtension) {
    // C/C++ extensions
    ASSERT_TRUE(echotrace::fs::has_supported_extension("main.cpp"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("header.hpp"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("code.cc"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("file.cxx"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("header.h"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("plain.c"));
    // Multi-language extensions added in v1.0
    ASSERT_TRUE(echotrace::fs::has_supported_extension("script.py"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("Main.java"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("app.js"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("index.ts"));
    // Non-source files must be rejected
    ASSERT_FALSE(echotrace::fs::has_supported_extension("data.txt"));
    ASSERT_FALSE(echotrace::fs::has_supported_extension("readme.md"));
    ASSERT_FALSE(echotrace::fs::has_supported_extension("image.png"));
    ASSERT_FALSE(echotrace::fs::has_supported_extension("Makefile"));
}

TEST(FsTraversalCaseInsensitive) {
    ASSERT_TRUE(echotrace::fs::has_supported_extension("FILE.CPP"));
    ASSERT_TRUE(echotrace::fs::has_supported_extension("FILE.Hpp"));
}

TEST(FsTraversalIgnoredDirectory) {
    ASSERT_TRUE(echotrace::fs::is_ignored_directory(".git"));
    ASSERT_TRUE(echotrace::fs::is_ignored_directory("build"));
    ASSERT_TRUE(echotrace::fs::is_ignored_directory("out"));
    ASSERT_TRUE(echotrace::fs::is_ignored_directory("bin"));
    ASSERT_TRUE(echotrace::fs::is_ignored_directory(".hidden"));
    ASSERT_TRUE(echotrace::fs::is_ignored_directory("cmake-build-debug"));
    ASSERT_FALSE(echotrace::fs::is_ignored_directory("src"));
    ASSERT_FALSE(echotrace::fs::is_ignored_directory("include"));
}

TEST(FsTraversalNonexistentThrows) {
    ASSERT_THROW(echotrace::fs::discover_sources("nonexistent_dir_12345"),
                 std::runtime_error);
}
