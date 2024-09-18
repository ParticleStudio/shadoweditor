export module behaviortree.delay_node;

import <atomic>;
import <string>;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {
/**
 * @brief The delay node will introduce a delay and then tick the
 * child returning the status of the GetChildNode as it is upon completion
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
export class BEHAVIORTREE_API DelayNode: public DecoratorNode {
 public:
    DelayNode(const std::string &rName, uint32_t milliseconds);

    DelayNode(const std::string &rName, const NodeConfig &rConfig);

    ~DelayNode() override {
        Halt();
    }

    static PortMap ProvidedPorts() {
        return {InputPort<uint32_t>("delay_msec", "Tick the GetChildNode after a few milliseconds")};
    }

    void Halt() override;

 private:
    TimerQueue<> m_timerQueue;
    uint64_t m_timerId;

    virtual behaviortree::NodeStatus Tick() override;

    bool m_delayStarted{false};
    std::atomic_bool m_delayComplete{false};
    bool m_delayAborted{false};
    uint32_t m_msec;
    bool m_readParameterFromPorts{false};
    std::mutex m_delayMutex;
};

}// namespace behaviortree

// module behaviortree.delay_node;
