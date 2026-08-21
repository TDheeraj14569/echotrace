#include "test_framework.hpp"
#include "echotrace/similarity.hpp"
#include <unordered_set>
#include <cstdint>

TEST(SimilarityIdentical100) {
    std::unordered_set<std::uint64_t> a = {1, 2, 3, 4};
    std::unordered_set<std::uint64_t> b = {1, 2, 3, 4};
    ASSERT_NEAR(echotrace::sim::jaccard(a, b), 100.0, 0.01);
}

TEST(SimilarityEmptySets100) {
    std::unordered_set<std::uint64_t> a;
    std::unordered_set<std::uint64_t> b;
    ASSERT_NEAR(echotrace::sim::jaccard(a, b), 100.0, 0.01);
}

TEST(SimilarityOneEmpty0) {
    std::unordered_set<std::uint64_t> a = {1, 2, 3};
    std::unordered_set<std::uint64_t> b;
    ASSERT_NEAR(echotrace::sim::jaccard(a, b), 0.0, 0.01);
}

TEST(SimilarityCompletelyDifferent) {
    std::unordered_set<std::uint64_t> a = {1, 2};
    std::unordered_set<std::uint64_t> b = {3, 4};
    ASSERT_NEAR(echotrace::sim::jaccard(a, b), 0.0, 0.01);
}

TEST(SimilarityPartialOverlap) {
    std::unordered_set<std::uint64_t> a = {1, 2, 3, 4};
    std::unordered_set<std::uint64_t> b = {3, 4, 5, 6};
    // intersection=2, union=6 -> 2/6*100 = 33.33%
    ASSERT_NEAR(echotrace::sim::jaccard(a, b), 33.33, 0.1);
}

TEST(SimilarityJaccardDetail) {
    std::unordered_set<std::uint64_t> a = {1, 2, 3};
    std::unordered_set<std::uint64_t> b = {3, 4, 5};
    auto detail = echotrace::sim::jaccard_detail(a, b);
    ASSERT_EQ(detail.intersection, static_cast<size_t>(1));
    ASSERT_EQ(detail.universe, static_cast<size_t>(5));
    ASSERT_NEAR(detail.percent, 20.0, 0.1);
}

TEST(SimilarityCompareConvenience) {
    auto score = echotrace::sim::compare(
        "int main() { return 0; }",
        "int main() { return 0; }",
        5, 4);
    ASSERT_NEAR(score, 100.0, 0.01);
}

TEST(SimilarityCompareDifferent) {
    auto score = echotrace::sim::compare(
        "void insertion_sort(int* d, int n) { for (int i=1; i<n; ++i) {} }",
        "#include <iostream>\nint main() { std::cout << 42; }",
        5, 4);
    ASSERT_LT(score, 50.0);
}
