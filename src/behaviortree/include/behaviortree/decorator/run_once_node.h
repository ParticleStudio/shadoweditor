#ifndef BEHAVIORTREE_RUN_ONCE_NODE_H
#define BEHAVIORTREE_RUN_ONCE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {

/**
 * @brief The RunOnceNode is used when you want to execute the child
 * only once.
 * If the child is asynchronous, we will tick until either SUCCESS or FAILURE is
 * returned.
 *
 * After that first execution, you can set value of the port "then_skip" to:
 *
 * - if TRUE (default), the node will be skipped in the future.
 * - if FALSE, return synchronously the same status returned by the child, forever.
 */
class RunOnceNode: public DecoratorNode {
 public:
    RunOnceNode(const std::string& refName, const NodeConfig& refConfig)
        : DecoratorNode(refName, refConfig) {
        SetRegistrationID("RunOnce");
    }

    static PortsList ProvidedPorts() {
        return {InputPort<bool>("then_skip", true,
                                "If true, skip after the first execution, "
                                "otherwise return the same NodeStatus returned once bu the "
                                "child.")};
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

    SetStatus(NodeStatus::RUNNING);
    const NodeStatus status = m_ChildNode->executeTick();

    if(isStatusCompleted(status)) {
        m_AlreadyTicked = true;
        m_ReturnedStatus = status;
        ResetChild();
    }
    return status;
}

}// namespace behaviortree

#endif// BEHAVIORTREE_RUN_ONCE_NODE_H
