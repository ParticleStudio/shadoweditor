export module behaviortree.force_success_node;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The ForceSuccessNode returns always SUCCESS or RUNNING.
 */
export class BEHAVIORTREE_API ForceSuccessNode: public DecoratorNode {
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

// module behaviortree.force_success_node;
