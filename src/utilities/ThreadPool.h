#pragma once

#include <future>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <type_traits>

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t size);
    ~ThreadPool();

    template<class F, typename... Args> requires(std::is_invocable_v<F, Args...>)
    auto Enqueue(F&& aFunc, Args&&... someArgs) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    void ThreadLoop();

    std::vector<std::jthread> myThreads;
    std::queue<std::function<void()>> myTasks;
    std::condition_variable myCV;
    std::mutex myMutex; // sync access to task queue
    bool myShutdown;
};

template<class F, typename... Args> requires(std::is_invocable_v<F, Args...>)
inline auto ThreadPool::Enqueue(F&& aFunc, Args&&... someArgs) -> std::future<std::invoke_result_t<F, Args...>>
{
    using ReturnType = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>
    (
        std::bind(std::forward<F>(aFunc), std::forward<Args>(someArgs)...)
    );

    std::future<ReturnType> result = task->get_future();
    {
        std::lock_guard<std::mutex> lock(myMutex); // lock to synchronize access to tasks

        if (myShutdown)
        {
            throw std::runtime_error("Thread pool has shut down, no more tasks can be added");
        }

        // place task in queue, copying the shared pointer keeps it alive
        myTasks.emplace([task] { (*task)(); });
    }

    myCV.notify_one(); // notify a thread that a task is available

    return result;
}

