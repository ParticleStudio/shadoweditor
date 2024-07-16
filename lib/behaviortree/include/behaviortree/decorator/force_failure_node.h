#ifndef BEHAVIORTREE_FORCE_FAILURE_NODE_H
#define BEHAVIORTREE_FORCE_FAILURE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The ForceFailureNode returns always FAILURE or RUNNING.
 */
class ForceFailureNode: public DecoratorNode {
 public:
    ForceFailureNode(const std::string &refName): DecoratorNode(refName, {}) {
        SetRegistrationId("ForceFailure");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus ForceFailureNode::Tick() {
    SetNodeStatus(NodeStatus::Running);

    const NodeStatus childStatus = m_ChildNode->ExecuteTick();

    if(IsNodeStatusCompleted(childStatus)) {
        ResetChild();
        return NodeStatus::Failure;
    }

    // RUNNING or skipping
    return childStatus;
}
}// namespace behaviortree

#endif// BEHAVIORTREE_FORCE_FAILURE_NODE_H
