#ifndef BEHAVIORTREE_TIMER_QUEUE_H
#define BEHAVIORTREE_TIMER_QUEUE_H

#include <assert.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace behaviortree {
// http://www.crazygaze.com/blog/2016/03/24/portable-c-timer-queue/

namespace details {
class Semaphore {
 public:
    Semaphore(uint32_t count = 0): m_Count(count) {}

    void Notify() {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Count++;
        }
        m_CV.notify_one();
    }

    template<class Clock, class Duration>
    bool WaitUntil(const std::chrono::time_point<Clock, Duration>& refPoint) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if(!m_CV.wait_until(lock, refPoint, [this]() {
               return m_Count > 0 || m_Unlock;
           })) {
            return false;
        }
        m_Count--;
        m_Unlock = false;
        return true;
    }

    void ManualUnlock() {
        m_Unlock = true;
        m_CV.notify_one();
    }

 private:
    std::mutex m_Mutex;
    std::condition_variable m_CV;
    unsigned m_Count{0};
    std::atomic_bool m_Unlock{false};
};
}// namespace details

// Timer Queue
//
// Allows execution of handlers at a specified time in the future
// Guarantees:
//  - All handlers are executed ONCE, even if canceled (aborted parameter will
//be set to true)
//      - If TimerQueue is destroyed, it will cancel all handlers.
//  - Handlers are ALWAYS executed in the Timer Queue worker thread.
//  - Handlers execution order is NOT guaranteed
//
template<
        typename _Clock = std::chrono::steady_clock,
        typename _Duration = std::chrono::steady_clock::duration>
class TimerQueue {
 public:
    TimerQueue() {
        m_Thread = std::thread([this] {
            Run();
        });
    }

    ~TimerQueue() {
        m_Finish = true;
        CancelAll();
        m_CheckWork.ManualUnlock();
        m_Thread.join();
    }

    //! Adds a new timer
    // \return
    //  Returns the ID of the new timer. You can use this ID to cancel the
    // timer
    uint64_t Add(
            std::chrono::milliseconds milliseconds,
            std::function<void(bool)> handler
    ) {
        WorkItem item;
        item.end = _Clock::now() + milliseconds;
        item.handler = std::move(handler);

        std::unique_lock<std::mutex> lk(m_Mutex);
        uint64_t id = ++m_IdCounter;
        item.id = id;
        m_Items.push(std::move(item));
        lk.unlock();

        // Something changed, so wake up timer thread
        m_CheckWork.Notify();
        return id;
    }

    //! Cancels the specified timer
    // \return
    //  1 if the timer was cancelled.
    //  0 if you were too late to cancel (or the timer ID was never valid to
    // start with)
    size_t Cancel(uint64_t id) {
        // Instead of removing the item from the container (thus breaking the
        // heap integrity), we set the item as having no handler, and put
        // that handler on a new item at the top for immediate execution
        // The timer thread will then ignore the original item, since it has no
        // handler.
        std::unique_lock<std::mutex> lk(m_Mutex);
        for(auto&& refItem: m_Items.GetContainer()) {
            if(refItem.id == id && refItem.handler) {
                WorkItem newItem;
                // Zero time, so it stays at the top for immediate execution
                newItem.end = std::chrono::time_point<_Clock, _Duration>();
                newItem.id = 0;// Means it is a canceled item
                // Move the handler from item to newItem.
                // Also, we need to manually set the handler to nullptr, since
                // the standard does not guarantee moving an std::function will
                // empty it. Some STL implementation will empty it, others will
                // not.
                newItem.handler = std::move(refItem.handler);
                refItem.handler = nullptr;
                m_Items.push(std::move(newItem));

                lk.unlock();
                // Something changed, so wake up timer thread
                m_CheckWork.Notify();
                return 1;
            }
        }
        return 0;
    }

    //! Cancels all timers
    // \return
    //  The number of timers cancelled
    size_t CancelAll() {
        // Setting all "end" to 0 (for immediate execution) is ok,
        // since it maintains the heap integrity
        std::unique_lock<std::mutex> lk(m_Mutex);
        for(auto&& refItem: m_Items.GetContainer()) {
            if(refItem.id) {
                refItem.end = std::chrono::time_point<_Clock, _Duration>();
                refItem.id = 0;
            }
        }
        auto ret = m_Items.size();

        lk.unlock();
        m_CheckWork.Notify();
        return ret;
    }

 private:
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    void Run() {
        while(!m_Finish) {
            auto end = CalcWaitTime();
            if(end.first) {
                // Timers found, so wait until it expires (or something else
                // changes)
                m_CheckWork.WaitUntil(end.second);
            } else {
                // No timers exist, so wait an arbitrary amount of time
                m_CheckWork.WaitUntil(
                        _Clock::now() + std::chrono::milliseconds(10)
                );
            }

            // Check and execute as much work as possible, such as, all expired
            // timers
            CheckWork();
        }

        // If we are shutting down, we should not have any items left,
        // since the shutdown cancels all items
        assert(m_Items.size() == 0);
    }

    std::pair<bool, std::chrono::time_point<_Clock, _Duration>> CalcWaitTime() {
        std::lock_guard<std::mutex> lk(m_Mutex);
        while(m_Items.size()) {
            if(m_Items.top().handler) {
                // Item present, so return the new wait time
                return std::make_pair(true, m_Items.top().end);
            } else {
                // Discard empty handlers (they were cancelled)
                m_Items.pop();
            }
        }

        // No items found, so return no wait time (causes the thread to wait
        // indefinitely)
        return std::make_pair(
                false, std::chrono::time_point<_Clock, _Duration>()
        );
    }

    void CheckWork() {
        std::unique_lock<std::mutex> lk(m_Mutex);
        while(m_Items.size() && m_Items.top().end <= _Clock::now()) {
            WorkItem item(std::move(m_Items.top()));
            m_Items.pop();

            lk.unlock();
            if(item.handler) {
                item.handler(item.id == 0);
            }
            lk.lock();
        }
    }

    details::Semaphore m_CheckWork;
    std::thread m_Thread;
    bool m_Finish = false;
    uint64_t m_IdCounter = 0;

    struct WorkItem {
        std::chrono::time_point<_Clock, _Duration> end;
        uint64_t id;// id==0 means it was cancelled
        std::function<void(bool)> handler;
        bool operator>(const WorkItem& other) const {
            return end > other.end;
        }
    };

    std::mutex m_Mutex;
    // Inheriting from priority_queue, so we can access the internal container
    class Queue: public std::priority_queue<
                         WorkItem, std::vector<WorkItem>, std::greater<WorkItem>> {
     public:
        std::vector<WorkItem>& GetContainer() {
            return this->c;
        }
    } m_Items;
};
}// namespace behaviortree

#endif// BEHAVIORTREE_TIMER_QUEUE_H
