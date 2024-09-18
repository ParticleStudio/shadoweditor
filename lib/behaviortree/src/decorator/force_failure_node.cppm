export module behaviortree.force_failure_node;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The ForceFailureNode returns always FAILURE or RUNNING.
 */
export class BEHAVIORTREE_API ForceFailureNode: public DecoratorNode {
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

// module behaviortree.force_failure_node;
