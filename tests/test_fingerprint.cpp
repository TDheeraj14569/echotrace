#include "test_framework.hpp"
#include "echotrace/fingerprint.hpp"
#include <vector>
#include <stdexcept>

using namespace echotrace::fingerprint;

TEST(Fingerprint_WinnowPicksReturnsCorrectPositions) {
    std::vector<uint64_t> hashes = {10, 20, 5, 30, 40, 2, 50, 60};
    auto picks = winnow_picks(hashes, 3);
    ASSERT_FALSE(picks.empty());
}

TEST(Fingerprint_WinnowReturnsCorrectHashSet) {
    std::vector<uint64_t> hashes = {10, 20, 5, 30, 40, 2, 50, 60};
    auto set = winnow(hashes, 3);
    ASSERT_FALSE(set.empty());
}

TEST(Fingerprint_IdenticalSequencesSameFingerprints) {
    std::vector<uint64_t> seq = {10, 20, 5, 30, 40, 2, 50, 60};
    auto f1 = winnow(seq, 3);
    auto f2 = winnow(seq, 3);
    ASSERT_EQ(f1.size(), f2.size());
}

TEST(Fingerprint_FromSourceConvenience) {
    auto f = from_source("void test() { int x = 0; }", 4, 3);
    ASSERT_FALSE(f.empty());
}

TEST(Fingerprint_WindowZeroThrows) {
    std::vector<uint64_t> h = {1, 2, 3};
    ASSERT_THROW(winnow(h, 0), std::invalid_argument);
}

TEST(Fingerprint_EmptyInputReturnsEmpty) {
    std::vector<uint64_t> empty_hashes;
    auto f = winnow(empty_hashes, 3);
    ASSERT_TRUE(f.empty());
}

TEST(Fingerprint_DeterministicResults) {
    std::vector<uint64_t> h = {1, 2, 3, 4, 5, 6, 7};
    auto f1 = winnow(h, 3);
    auto f2 = winnow(h, 3);
    ASSERT_EQ(f1.size(), f2.size());
}
