#ifndef BEHAVIORTREE_FORCE_FAILURE_NODE_H
#define BEHAVIORTREE_FORCE_FAILURE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The ForceFailureNode returns always FAILURE or RUNNING.
 */
class ForceFailureNode: public DecoratorNode {
 public:
    ForceFailureNode(const std::string &rName): DecoratorNode(rName, {}) {
        SetRegistrationId("ForceFailure");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus ForceFailureNode::Tick() {
    SetNodeStatus(NodeStatus::Running);

    const NodeStatus childStatus = m_childNode->ExecuteTick();

    if(IsNodeStatusCompleted(childStatus)) {
        ResetChildNode();
        return NodeStatus::Failure;
    }

    // RUNNING or skipping
    return childStatus;
}
}// namespace behaviortree

#endif// BEHAVIORTREE_FORCE_FAILURE_NODE_H
