#ifndef BEHAVIORTREE_FORCE_SUCCESS_NODE_H
#define BEHAVIORTREE_FORCE_SUCCESS_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The ForceSuccessNode returns always SUCCESS or RUNNING.
 */
class ForceSuccessNode: public DecoratorNode {
 public:
    ForceSuccessNode(const std::string &rName): DecoratorNode(rName, {}) {
        SetRegistrationId("ForceSuccess");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus ForceSuccessNode::Tick() {
    SetNodeStatus(NodeStatus::Running);

    const NodeStatus childStatus = m_childNode->ExecuteTick();

    if(IsNodeStatusCompleted(childStatus)) {
        ResetChildNode();
        return NodeStatus::Success;
    }

    // RUNNING or skipping
    return childStatus;
}
}// namespace behaviortree

#endif// BEHAVIORTREE_FORCE_SUCCESS_NODE_H
