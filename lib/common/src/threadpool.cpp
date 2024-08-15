#include "common/threadpool.hpp"

namespace common {
ThreadPool::ThreadPool(Singleton<ThreadPool>::Token) {
}

ThreadPool::~ThreadPool() {
    Wait();
    Release();
}

void ThreadPool::Init() {
    Init(0, [] {});
}

void ThreadPool::Init(const concurrency_t threadNum) {
    Init(threadNum, [] {});
}

void ThreadPool::Init(const concurrency_t threadNum, const std::function<void()> &rInitTask) {
    this->m_threadNum = DetermineThreadNum(threadNum);
    this->m_pThreads = std::make_unique<std::thread[]>(DetermineThreadNum(threadNum));
    Init(rInitTask);
}

void ThreadPool::Init(const std::function<void()> &rInitTask) {
    {
        const std::scoped_lock lock(m_taskMutex);
        m_runningTaskNum = m_threadNum;
        m_isRunning = true;
    }

    for(concurrency_t i = 0; i < m_threadNum; ++i) {
        m_pThreads[i] = std::thread(&ThreadPool::Run, this, i, rInitTask);
    }
}

void ThreadPool::Release() {
    {
        const std::scoped_lock lock(m_taskMutex);
        m_isRunning = false;
    }

    m_taskAvailableCV.notify_all();

    for(concurrency_t i = 0; i < m_threadNum; ++i) {
        m_pThreads[i].join();
    }
}

std::vector<std::thread::native_handle_type> ThreadPool::GetNativeHandle() const {
    std::vector<std::thread::native_handle_type> nativeHandleVec(this->m_threadNum);
    for(concurrency_t i = 0; i < this->m_threadNum; ++i) {
        nativeHandleVec[i] = this->m_pThreads[i].native_handle();
    }
    return nativeHandleVec;
}

std::size_t ThreadPool::GetTaskQueued() const {
    const std::scoped_lock lock(m_taskMutex);
    return m_taskQueue.size();
}

std::size_t ThreadPool::GetRunningTaskNum() const {
    const std::scoped_lock lock(m_taskMutex);
    return m_runningTaskNum;
}

std::size_t ThreadPool::GetTaskNum() const {
    const std::scoped_lock lock(m_taskMutex);
    return m_runningTaskNum + m_taskQueue.size();
}

concurrency_t ThreadPool::GetThreadNum() const {
    return m_threadNum;
}

std::vector<std::thread::id> ThreadPool::GetThreadIds() const {
    std::vector<std::thread::id> threadIds(m_threadNum);
    for(concurrency_t i = 0; i < m_threadNum; ++i) {
        threadIds[i] = m_pThreads[i].get_id();
    }
    return threadIds;
}

bool ThreadPool::IsPaused() const {
    const std::scoped_lock lock(m_taskMutex);
    return m_isPaused;
}

void ThreadPool::Pause() {
    const std::scoped_lock lock(m_taskMutex);
    m_isPaused = true;
}

void ThreadPool::Purge() {
    const std::scoped_lock lock(m_taskMutex);
    while(!m_taskQueue.empty()) {
        m_taskQueue.pop();
    }
}

void ThreadPool::Reset() {
    Reset(0, [] {});
}

void ThreadPool::Reset(const concurrency_t threadsNum) {
    Reset(threadsNum, [] {});
}

void ThreadPool::Reset(const std::function<void()> &rInitTaskFunc) {
    Reset(0, rInitTaskFunc);
}

void ThreadPool::Reset(const concurrency_t threadsNum, const std::function<void()> &rInitTaskFunc) {
#ifdef THREAD_POOL_ENABLE_PAUSE
    std::unique_lock lock(m_mutex);
    const bool isPaused = m_isPaused;
    m_isPaused = true;
    lock.unlock();
#endif
    Wait();
    Release();
    m_threadNum = DetermineThreadNum(threadsNum);
    m_pThreads = std::make_unique<std::thread[]>(m_threadNum);
    Init(rInitTaskFunc);
#ifdef THREAD_POOL_ENABLE_PAUSE
    lock.lock();
    m_isPaused = isPaused;
#endif
}

void ThreadPool::Resume() {
    {
        const std::scoped_lock lock(m_taskMutex);
        m_isPaused = false;
    }
    m_taskAvailableCV.notify_all();
}

void ThreadPool::Wait() {
#ifdef THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK
    if(this_thread::get_pool() == this)
        throw WaitDeadlock();
#endif
    std::unique_lock lock(m_taskMutex);
    m_isWaiting = true;
    m_taskDoneCV.wait(lock, [this] {
        return (m_runningTaskNum == 0) && THREAD_POOL_PAUSED_OR_EMPTY;
    });
    m_isWaiting = false;
}

concurrency_t ThreadPool::DetermineThreadNum(const concurrency_t threadsNum) {
    if(threadsNum > 0) {
        return threadsNum;
    }

    if(std::thread::hardware_concurrency() > 0) {
        return std::thread::hardware_concurrency();
    }

    return 1;
}

void ThreadPool::Run(const concurrency_t idx, const std::function<void()> &rInitTaskFunc) {
    this_thread::GetIndex.m_index = idx;
    this_thread::GetPool.m_pool = this;
    rInitTaskFunc();
    std::unique_lock lock(m_taskMutex);
    while(true) {
        --m_runningTaskNum;
        lock.unlock();
        if(m_isWaiting && (m_runningTaskNum == 0) && THREAD_POOL_PAUSED_OR_EMPTY) {
            m_taskDoneCV.notify_all();
        }
        lock.lock();

        m_taskAvailableCV.wait(lock, [this] {
            return !THREAD_POOL_PAUSED_OR_EMPTY || !m_isRunning;
        });

        if(!m_isRunning) {
            break;
        }

        {
#ifdef THREAD_POOL_ENABLE_PRIORITY
            const std::function<void()> task = std::move(std::remove_const_t<pr_task &>(m_taskQueue.top()).task);
            m_taskQueue.pop();
#else
            const std::function<void()> task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
#endif
            ++m_runningTaskNum;
            lock.unlock();
            task();
        }
        lock.lock();
    }
    this_thread::GetIndex.m_index = std::nullopt;
    this_thread::GetPool.m_pool = std::nullopt;
}

namespace this_thread {
OptionalPool ThreadInfoPool::operator()() const {
    return m_pool;
}
}// namespace this_thread

}// namespace common