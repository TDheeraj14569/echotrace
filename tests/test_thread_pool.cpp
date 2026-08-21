#include "test_framework.hpp"
#include "echotrace/thread_pool.hpp"
#include <atomic>
#include <stdexcept>
#include <vector>
#include <future>

TEST(ThreadPoolSingleTask) {
    echotrace::ThreadPool pool(2);
    auto fut = pool.submit([]{ return 42; });
    ASSERT_EQ(fut.get(), 42);
}

TEST(ThreadPoolMultipleTasks) {
    echotrace::ThreadPool pool(4);
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.submit([&counter]{ counter.fetch_add(1); }));
    }
    for (auto& f : futures) {
        f.get();
    }
    ASSERT_EQ(counter.load(), 10);
}

TEST(ThreadPoolResults) {
    echotrace::ThreadPool pool(2);
    auto f1 = pool.submit([]{ return 10; });
    auto f2 = pool.submit([]{ return 20; });
    ASSERT_EQ(f1.get() + f2.get(), 30);
}

TEST(ThreadPoolZeroThrows) {
    ASSERT_THROW(echotrace::ThreadPool(0), std::invalid_argument);
}

TEST(ThreadPoolExceptionPropagation) {
    echotrace::ThreadPool pool(2);
    auto f = pool.submit([]() -> int { throw std::runtime_error("test error"); });
    ASSERT_THROW(f.get(), std::runtime_error);
}

TEST(ThreadPoolConcurrentNoInterference) {
    echotrace::ThreadPool pool(4);
    std::atomic<int> sum{0};
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool.submit([&sum]{ sum.fetch_add(1, std::memory_order_relaxed); }));
    }
    for (auto& f : futures) {
        f.get();
    }
    ASSERT_EQ(sum.load(), 100);
}

TEST(ThreadPoolWorkerCount) {
    echotrace::ThreadPool pool(3);
    ASSERT_EQ(pool.worker_count(), static_cast<size_t>(3));
}
