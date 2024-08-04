#include "common/threadpool.hpp"

namespace common {
ThreadPool::ThreadPool(Singleton<ThreadPool>::Token): m_isRunning(true) {
}

void ThreadPool::Init(ThreadNum_t threadNum) {
    for(ThreadNum_t i = 0; i < threadNum; i++)
        this->m_workerVec.emplace_back([this](ThreadNum_t n) {
            while(true) {
                std::function<void()> taskFunc;
                {
                    std::unique_lock<std::mutex> lock(this->m_queueMutex);
                    this->m_condition.wait(lock, [this] {
                        return !this->m_isRunning || !this->m_taskQueue.empty();
                    });

                    if(!this->m_isRunning && this->m_taskQueue.empty()) {
                        break;
                    }

                    taskFunc = std::move(this->m_taskQueue.front());
                    this->m_taskQueue.pop();
                }
                taskFunc();
            }
        }, i);
}

void ThreadPool::JoinAll() {
    for(std::thread &rWorker: this->m_workerVec)
        if(rWorker.joinable()) {
            rWorker.join();
        }
}

void ThreadPool::Release() {
    this->m_isRunning = false;
    this->m_isRunning.notify_all();
}
}// namespace common
