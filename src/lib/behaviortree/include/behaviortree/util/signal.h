#ifndef BEHAVIORTREE_SIGNAL_H
#define BEHAVIORTREE_SIGNAL_H

#include <functional>
#include <memory>
#include <vector>

namespace behaviortree {
/**
 * Super simple Signal/Slop implementation, AKA "Observable pattern".
 * The subscriber is active until it goes out of scope or Subscriber::reset() is called.
 */
template<typename... CallableArgs>
class Signal {
 public:
    using CallableFunction = std::function<void(CallableArgs...)>;
    using Subscriber = std::shared_ptr<CallableFunction>;

    void notify(CallableArgs... args) {
        for(size_t i = 0; i < m_Subscribers.size();) {
            if(auto sub = m_Subscribers[i].lock()) {
                (*sub)(args...);
                i++;
            } else {
                m_Subscribers.erase(m_Subscribers.begin() + i);
            }
        }
    }

    Subscriber Subscribe(CallableFunction func) {
        Subscriber sub = std::make_shared<CallableFunction>(std::move(func));
        m_Subscribers.emplace_back(sub);
        return sub;
    }

 private:
    std::vector<std::weak_ptr<CallableFunction>> m_Subscribers;
};
}// namespace behaviortree

#endif// BEHAVIORTREE_SIGNAL_H
