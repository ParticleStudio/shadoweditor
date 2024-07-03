#ifndef BEHAVIORTREE_TIMEOUT_NODE_H
#define BEHAVIORTREE_TIMEOUT_NODE_H

#include <atomic>

#include "behaviortree/decorator_node.h"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {
/**
 * @brief The TimeoutNode will halt() a running child if
 * the latter has been RUNNING longer than a given time.
 * The timeout is in milliseconds and it is passed using the port "msec".
 *
 * If timeout is reached, the node returns FAILURE.
 *
 * Example:
 *
 * <Timeout msec="5000">
 *    <KeepYourBreath/>
 * </Timeout>
 */

class TimeoutNode: public DecoratorNode {
 public:
    TimeoutNode(const std::string& refName, uint32_t milliseconds)
        : DecoratorNode(refName, {}), m_ChildHalted(false), m_TimerId(0), m_Msec(milliseconds), m_ReadParameterFromPorts(false), m_TimeoutStarted(false) {
        SetRegistrationID("Timeout");
    }

    TimeoutNode(const std::string& refName, const NodeConfig& refConfig)
        : DecoratorNode(refName, refConfig), m_ChildHalted(false), m_TimerId(0), m_Msec(0), m_ReadParameterFromPorts(true), m_TimeoutStarted(false) {}

    ~TimeoutNode() override {
        m_TimerQueue.CancelAll();
    }

    static PortsList providedPorts() {
        return {InputPort<unsigned>("msec",
                                    "After a certain amount of time, "
                                    "halt() the child if it is still running.")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;

    void Halt() override;

    TimerQueue<> m_TimerQueue;
    std::atomic_bool m_ChildHalted{false};
    uint64_t c;

    uint32_t m_Msec;
    bool m_ReadParameterFromPorts;
    std::atomic_bool m_TimeoutStarted{false};
    std::mutex m_TimeoutMutex;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_TIMEOUT_NODE_H
