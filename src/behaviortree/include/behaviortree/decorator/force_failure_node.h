#ifndef BEHAVIORTREE_FORCE_FAILURE_NODE_H
#define BEHAVIORTREE_FORCE_FAILURE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The ForceFailureNode returns always FAILURE or RUNNING.
 */
class ForceFailureNode: public DecoratorNode {
 public:
    ForceFailureNode(const std::string& refName): DecoratorNode(refName, {}) {
        SetRegistrationID("ForceFailure");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus ForceFailureNode::Tick() {
    SetStatus(NodeStatus::RUNNING);

    const NodeStatus childStatus = m_ChildNode->executeTick();

    if(IsStatusCompleted(childStatus)) {
        ResetChild();
        return NodeStatus::FAILURE;
    }

    // RUNNING or skipping
    return childStatus;
}
}// namespace behaviortree

#endif// BEHAVIORTREE_FORCE_FAILURE_NODE_H
