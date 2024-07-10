#ifndef BEHAVIORTREE_SLEEP_NODE_H
#define BEHAVIORTREE_SLEEP_NODE_H

#include <atomic>

#include "behaviortree/action_node.h"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {
/**
 * @brief Sleep for a certain amount of time.
 * Consider also using the decorator <Delay/>
 *
 * <Sleep msec="5000"/>
 */
class SleepNode: public StatefulActionNode {
 public:
    SleepNode(const std::string &refName, const NodeConfig &refConfig);

    ~SleepNode() override {
        Halt();
    }

    NodeStatus OnStart() override;

    NodeStatus OnRunning() override;

    void OnHalted() override;

    static PortsList ProvidedPorts() {
        return {InputPort<unsigned>("msec")};
    }

 private:
    TimerQueue<> m_TimerQueue;
    uint64_t m_TimerId;

    std::atomic_bool m_TimerWaiting{false};
    std::mutex m_DelayMutex;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SLEEP_NODE_H
