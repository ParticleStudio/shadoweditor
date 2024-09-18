export module behaviortree.keep_running_until_failure_node;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The KeepRunningUntilFailureNode returns always FAILURE or RUNNING.
 */
export class BEHAVIORTREE_API KeepRunningUntilFailureNode: public DecoratorNode {
 public:
    KeepRunningUntilFailureNode(const std::string &rName): DecoratorNode(rName, {}) {
        SetRegistrationId("KeepRunningUntilFailure");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus KeepRunningUntilFailureNode::Tick() {
    SetNodeStatus(NodeStatus::Running);

    const NodeStatus childState = m_childNode->ExecuteTick();

    switch(childState) {
        case NodeStatus::Failure: {
            ResetChildNode();
            return NodeStatus::Failure;
        }
        case NodeStatus::Success: {
            ResetChildNode();
            return NodeStatus::Running;
        }
        case NodeStatus::Running: {
            return NodeStatus::Running;
        }
        default: {
            // TODO throw?
        } break;
    }
    return GetNodeStatus();
}
}// namespace behaviortree

// module behaviortree.keep_running_until_failure_node;
