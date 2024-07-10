#ifndef BEHAVIORTREE_DELAY_NODE_H
#define BEHAVIORTREE_DELAY_NODE_H

#include <atomic>

#include "behaviortree/decorator_node.h"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {
/**
 * @brief The delay node will introduce a delay and then tick the
 * child returning the status of the GetChild as it is upon completion
 * The delay is in milliseconds and it is passed using the port "delay_msec".
 *
 * During the delay the node changes status to RUNNING
 *
 * Example:
 *
 * <Delay delay_msec="5000">
 *    <KeepYourBreath/>
 * </Delay>
 */
class DelayNode: public DecoratorNode {
 public:
    DelayNode(const std::string& refName, uint32_t milliseconds);

    DelayNode(const std::string& refName, const NodeConfig& refConfig);

    ~DelayNode() override {
        Halt();
    }

    static PortsList ProvidedPorts() {
        return {InputPort<uint32_t>(
                "delay_msec",
                "Tick the GetChild after a few "
                "milliseconds"
        )};
    }

    void Halt() override;

 private:
    TimerQueue<> m_TimerQueue;
    uint64_t m_TimerId;

    virtual behaviortree::NodeStatus Tick() override;

    bool m_DelayStarted{false};
    std::atomic_bool m_DelayComplete{false};
    bool m_DelayAborted{false};
    uint32_t m_Msec;
    bool m_ReadParameterFromPorts{false};
    std::mutex m_DelayMutex;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_DELAY_NODE_H
