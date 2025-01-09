module;

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <future>

#include "common/common.h"

export module shadow.thread.pool;

namespace shadow::thread {
export COMMON_API class Pool {
public:
    Pool() {}

    ~Pool() noexcept {
        this->Stop();
    }

    void Init(const uint32_t numThreads) {
        if (numThreads > 0) {
            this->m_numThread = numThreads;
        }

        this->m_running = true;
        for(uint32_t i = 0; i < this->m_numThread; ++i) {
            m_workerVec.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(this->m_queueMutex);
                        // 等待任务或停止信号
                        this->m_cv.wait(lock, [this] {
                            return !this->m_running || !this->m_taskQue.empty();
                        });
                        if(!this->m_running && this->m_taskQue.empty()) {
                            return;
                        }
                        // 从队列中取出任务
                        task = std::move(this->m_taskQue.front());
                        this->m_taskQue.pop();
                    }
                    // 执行任务
                    task();
                }
            });
        }
    }

    void JoinAll() {
        for(std::thread &worker: m_workerVec) {
            if(worker.joinable()) {
                worker.join();
            }
        }
    }

    void Stop() {
        if(!this->m_running) {
            return;
        }

        {
            const std::scoped_lock<std::mutex> lock(m_queueMutex);
            this->m_running = false;
        }
        this->JoinAll();
        // 通知所有线程停止
        this->m_cv.notify_all();
    }

    // 向线程池添加任务，返回一个 future 用于获取任务的结果
    template<class Func, class... Args>
    auto AddTask(Func &&func, Args &&... args) -> std::future<std::invoke_result_t<Func, Args...>> {
        using ReturnType = std::invoke_result_t<Func, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<ReturnType> result = task->get_future();
        {
            const std::scoped_lock lock(m_queueMutex);
            // 不允许在停止后添加任务
            if(!this->m_running) {
                throw std::runtime_error("add task on stopped ThreadPool");
            }
            this->m_taskQue.emplace([task]() { (*task)(); });
        }
        // 通知一个线程有新任务
        this->m_cv.notify_one();
        return result;
    }

private:
    // 存储工作线程
    std::vector<std::thread> m_workerVec{};
    // 存储任务队列
    std::queue<std::function<void()>> m_taskQue{};
    // 互斥锁，用于保护任务队列
    std::mutex m_queueMutex;
    // 条件变量，用于线程间的通知和等待
    std::condition_variable m_cv;
    // 停止标志
    bool m_running{false};
    uint32_t m_numThread{std::thread::hardware_concurrency()};
};
} // namespace shadow::thread

// module shadow.thread.pool;
// module;
