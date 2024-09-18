export module behaviortree.sleep_node;

import <atomic>;

import behaviortree.action_node;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {
/**
 * @brief Sleep for a certain amount of time.
 * Consider also using the decorator <Delay/>
 *
 * <Sleep msec="5000"/>
 */
export class BEHAVIORTREE_API SleepNode: public StatefulActionNode {
 public:
    SleepNode(const std::string &rName, const NodeConfig &rConfig);

    ~SleepNode() override {
        Halt();
    }

    NodeStatus OnStart() override;

    NodeStatus OnRunning() override;

    void OnHalted() override;

    static PortMap ProvidedPorts() {
        return {InputPort<unsigned>("msec")};
    }

 private:
    TimerQueue<> m_timerQueue;
    uint64_t m_timerId;

    std::atomic_bool m_timerWaiting{false};
    std::mutex m_delayMutex;
};

}// namespace behaviortree

// module behaviortree.sleep_node;
