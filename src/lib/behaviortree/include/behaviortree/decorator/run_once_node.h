#ifndef BEHAVIORTREE_RUN_ONCE_NODE_H
#define BEHAVIORTREE_RUN_ONCE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {

/**
 * @brief The RunOnceNode is used when you want to execute the GetChild
 * only once.
 * If the GetChild is asynchronous, we will tick until either SUCCESS or FAILURE is
 * returned.
 *
 * After that first execution, you can set value of the port "then_skip" to:
 *
 * - if TRUE (default), the node will be skipped in the future.
 * - if FALSE, return synchronously the same status returned by the GetChild, forever.
 */
class RunOnceNode: public DecoratorNode {
 public:
    RunOnceNode(const std::string& refName, const NodeConfig& refConfig)
        : DecoratorNode(refName, refConfig) {
        SetRegistrationId("RunOnce");
    }

    static PortsList ProvidedPorts() {
        return {InputPort<bool>("then_skip", true,
                                "If true, skip after the first execution, "
                                "otherwise return the same NodeStatus returned once bu the "
                                "GetChild.")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;

    bool m_AlreadyTicked{false};
    NodeStatus m_ReturnedStatus{NodeStatus::IDLE};
};

//------------ implementation ----------------------------

inline NodeStatus RunOnceNode::Tick() {
    bool skip{true};
    if(auto const res = GetInput<bool>("then_skip")) {
        skip = res.value();
    }

    if(m_AlreadyTicked) {
        return skip ? NodeStatus::SKIPPED : m_ReturnedStatus;
    }

    SetNodeStatus(NodeStatus::RUNNING);
    const NodeStatus nodeStatus = m_ChildNode->ExecuteTick();

    if(IsStatusCompleted(nodeStatus)) {
        m_AlreadyTicked = true;
        m_ReturnedStatus = nodeStatus;
        ResetChild();
    }
    return nodeStatus;
}

}// namespace behaviortree

#endif// BEHAVIORTREE_RUN_ONCE_NODE_H
