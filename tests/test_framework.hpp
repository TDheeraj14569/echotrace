#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <stdexcept>
#include <exception>

namespace test_framework {
    struct TestInfo {
        std::string name;
        std::function<void()> func;
    };

    inline std::vector<TestInfo>& get_tests() {
        static std::vector<TestInfo> tests;
        return tests;
    }

    struct RegisterTest {
        RegisterTest(const std::string& name, std::function<void()> func) {
            get_tests().push_back({name, func});
        }
    };

    inline int run_all_tests() {
        int passed = 0;
        int failed = 0;

        for (const auto& test : get_tests()) {
            std::cout << "[ RUN      ] " << test.name << "\n";
            try {
                test.func();
                std::cout << "[       OK ] " << test.name << "\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "[  FAILED  ] " << test.name << " (Exception: " << e.what() << ")\n";
                failed++;
            } catch (...) {
                std::cout << "[  FAILED  ] " << test.name << " (Unknown exception)\n";
                failed++;
            }
        }

        std::cout << "\n=========================================\n";
        std::cout << "Tests run: " << (passed + failed) << "\n";
        std::cout << "Passed:    " << passed << "\n";
        std::cout << "Failed:    " << failed << "\n";

        if (failed == 0) {
            std::cout << "\nSUCCESS: All tests passed.\n";
            return 0;
        } else {
            std::cout << "\nFAILURE: Some tests failed.\n";
            return 1;
        }
    }

    inline void assert_true(bool expr, const char* expr_str, const char* file, int line) {
        if (!expr) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + expr_str + " is false");
        }
    }

    inline void assert_false(bool expr, const char* expr_str, const char* file, int line) {
        if (expr) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + expr_str + " is true");
        }
    }

    template<typename T1, typename T2>
    inline void assert_eq(const T1& a, const T2& b, const char* a_str, const char* b_str, const char* file, int line) {
        if (!(a == b)) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " == " + b_str);
        }
    }

    template<typename T1, typename T2>
    inline void assert_ne(const T1& a, const T2& b, const char* a_str, const char* b_str, const char* file, int line) {
        if (!(a != b)) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " != " + b_str);
        }
    }

    template<typename T1, typename T2>
    inline void assert_gt(const T1& a, const T2& b, const char* a_str, const char* b_str, const char* file, int line) {
        if (!(a > b)) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " > " + b_str);
        }
    }

    template<typename T1, typename T2>
    inline void assert_ge(const T1& a, const T2& b, const char* a_str, const char* b_str, const char* file, int line) {
        if (!(a >= b)) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " >= " + b_str);
        }
    }

    template<typename T1, typename T2>
    inline void assert_lt(const T1& a, const T2& b, const char* a_str, const char* b_str, const char* file, int line) {
        if (!(a < b)) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " < " + b_str);
        }
    }

    template<typename T1, typename T2>
    inline void assert_le(const T1& a, const T2& b, const char* a_str, const char* b_str, const char* file, int line) {
        if (!(a <= b)) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " <= " + b_str);
        }
    }

    template<typename T1, typename T2>
    inline void assert_near(const T1& a, const T2& b, double eps, const char* a_str, const char* b_str, const char* file, int line) {
        if (std::abs(static_cast<double>(a) - static_cast<double>(b)) > eps) {
            throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": Assertion failed: " + a_str + " is near " + b_str);
        }
    }
}

#define TEST(name) \
    void test_func_##name(); \
    static test_framework::RegisterTest reg_##name(#name, test_func_##name); \
    void test_func_##name()

#define ASSERT_TRUE(expr) test_framework::assert_true((expr), #expr, __FILE__, __LINE__)
#define ASSERT_FALSE(expr) test_framework::assert_false((expr), #expr, __FILE__, __LINE__)
#define ASSERT_EQ(a, b) test_framework::assert_eq((a), (b), #a, #b, __FILE__, __LINE__)
#define ASSERT_NE(a, b) test_framework::assert_ne((a), (b), #a, #b, __FILE__, __LINE__)
#define ASSERT_GT(a, b) test_framework::assert_gt((a), (b), #a, #b, __FILE__, __LINE__)
#define ASSERT_GE(a, b) test_framework::assert_ge((a), (b), #a, #b, __FILE__, __LINE__)
#define ASSERT_LT(a, b) test_framework::assert_lt((a), (b), #a, #b, __FILE__, __LINE__)
#define ASSERT_LE(a, b) test_framework::assert_le((a), (b), #a, #b, __FILE__, __LINE__)
#define ASSERT_NEAR(a, b, eps) test_framework::assert_near((a), (b), (eps), #a, #b, __FILE__, __LINE__)

#define ASSERT_THROW(expr, type) \
    do { \
        bool threw = false; \
        try { \
            expr; \
        } catch (const type&) { \
            threw = true; \
        } catch (...) { \
        } \
        if (!threw) { \
            throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": Assertion failed: expected exception of type " #type " not thrown by " #expr); \
        } \
    } while (false)

#define RUN_ALL_TESTS() test_framework::run_all_tests()
