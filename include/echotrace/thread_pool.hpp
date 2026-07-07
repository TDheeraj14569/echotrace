#ifndef ECHOTRACE_THREAD_POOL_HPP
#define ECHOTRACE_THREAD_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace echotrace
{

/// A fixed-size pool of worker threads executing arbitrary callables.
///
/// Design notes:
///   * Workers are created once and reused for the pool's lifetime.
///   * submit() returns a std::future so callers can synchronise per task.
///   * The pool never touches caller data; tasks own their inputs/outputs.
///     This keeps the comparison engine free of shared mutable state and makes
///     output ordering depend only on how results are collected, not on the
///     schedule in which tasks complete (see analysis::compare_all).
class ThreadPool
{
public:
    /// Create [threads] workers. A count of 0 throws std::invalid_argument.
    explicit ThreadPool(std::size_t threads)
    {
        if (threads == 0)
        {
            throw std::invalid_argument("ThreadPool requires at least one thread");
        }
        workers_.reserve(threads);
        for (std::size_t i = 0; i < threads; ++i)
        {
            workers_.emplace_back([this] { workerLoop(); });
        }
    }

    /// Join all workers on destruction. Tasks already queued still run.
    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_)
        {
            if (w.joinable())
            {
                w.join();
            }
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /// Enqueue a callable, returning a future for its result.
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using R = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<R()>>(
            [f = std::forward<F>(f), args_tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable -> R
            {
                return std::apply(std::move(f), std::move(args_tuple));
            });

        std::future<R> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_)
            {
                throw std::runtime_error("cannot submit to a stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

    /// Number of worker threads in the pool.
    std::size_t worker_count() const noexcept
    {
        return workers_.size();
    }

private:
    void workerLoop()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty())
                {
                    return;
                }
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};

} // namespace echotrace

#endif // ECHOTRACE_THREAD_POOL_HPP
