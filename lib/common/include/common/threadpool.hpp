#ifndef COMMON_THREADPOOL_H
#define COMMON_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

#include "common/singleton.h"

namespace common {
using ThreadNum_t = uint32_t;

const ThreadNum_t WORK_THREAD_NUM = 3;

class ThreadPool final: public common::Singleton<ThreadPool> {
 public:
    ThreadPool() = delete;

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

    explicit ThreadPool(Singleton<ThreadPool>::Token);

    ~ThreadPool() noexcept override = default;

    void Init(ThreadNum_t);

    void Release();

    template<class F, class... Args>
    auto AddTask(F &&rFunc, Args &&...args) -> std::future<std::invoke_result_t<decltype(rFunc), Args...>>;

    void JoinAll();

 private:
    std::vector<std::jthread> m_workerVec;

    std::queue<std::function<void()>> m_taskQueue;

    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_isRunning;
};

template<class F, class... Args>
auto ThreadPool::AddTask(F &&rFunc, Args &&...args) -> std::future<std::invoke_result_t<decltype(rFunc), Args...>> {
    using ReturnType = std::invoke_result_t<decltype(rFunc), Args...>;

    auto pTask = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(rFunc), std::forward<Args>(args)...));

    std::future<ReturnType> ret = pTask->get_future();

    {
        const std::unique_lock<std::mutex> lock(this->m_queueMutex);

        // don't allow adding task when the pool not running
        if(!this->m_isRunning) {
            throw std::runtime_error("add thread task error, threadpool is not running");
        }

        this->m_taskQueue.emplace([pTask]() {
            (*pTask)();
        });
    }

    this->m_condition.notify_one();

    return ret;
}
}// namespace common

#endif// COMMON_THREADPOOL_H
