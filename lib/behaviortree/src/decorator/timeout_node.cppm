export module behaviortree.timeout_node;

import <atomic>;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {
/**
 * @brief The TimeoutNode will halt() a running GetChildNode if
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

export class BEHAVIORTREE_API TimeoutNode: public DecoratorNode {
 public:
    TimeoutNode(const std::string &rName, uint32_t milliseconds): DecoratorNode(rName, {}),
                                                                  m_childHalted(false),
                                                                  m_timerId(0),
                                                                  m_msec(milliseconds),
                                                                  m_readParameterFromPorts(false),
                                                                  m_timeoutStarted(false) {
        SetRegistrationId("Timeout");
    }

    TimeoutNode(const std::string &rName, const NodeConfig &rConfig): DecoratorNode(rName, rConfig),
                                                                      m_childHalted(false),
                                                                      m_timerId(0),
                                                                      m_msec(0),
                                                                      m_readParameterFromPorts(true),
                                                                      m_timeoutStarted(false) {}

    ~TimeoutNode() override {
        m_timerQueue.CancelAll();
    }

    static PortMap ProvidedPorts() {
        return {InputPort<unsigned>("msec", "After a certain amount of time, halt() the GetChildNode if it is still running.")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;

    void Halt() override;

    TimerQueue<> m_timerQueue;
    std::atomic_bool m_childHalted{false};
    uint64_t m_timerId;

    uint32_t m_msec;
    bool m_readParameterFromPorts;
    std::atomic_bool m_timeoutStarted{false};
    std::mutex m_timeoutMutex;
};

}// namespace behaviortree

// module behaviortree.timeout_node;
