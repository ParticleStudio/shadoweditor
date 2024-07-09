#ifndef BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP
#define BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace behaviortree {

class WakeUpSignal {
 public:
    /// Return true if the timeout was NOT reached and the
    /// signal was received.
    bool WaitFor(std::chrono::microseconds usec) {
        std::unique_lock<std::mutex> lk(m_Mutex);
        auto res = m_CV.wait_for(lk, usec, [this] { return m_Ready.load(); });
        m_Ready = false;
        return res;
    }

    void EmitSignal() {
        m_Ready = true;
        m_CV.notify_all();
    }

 private:
    std::mutex m_Mutex;
    std::condition_variable m_CV;
    std::atomic_bool m_Ready{false};
};

}// namespace behaviortree

#endif// BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP
