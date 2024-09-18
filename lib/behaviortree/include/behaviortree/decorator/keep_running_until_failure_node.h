#ifndef BEHAVIORTREE_KEEP_RUNNING_UNTIL_FAILURE_NODE_H
#define BEHAVIORTREE_KEEP_RUNNING_UNTIL_FAILURE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The KeepRunningUntilFailureNode returns always FAILURE or RUNNING.
 */
class KeepRunningUntilFailureNode: public DecoratorNode {
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

#endif// BEHAVIORTREE_KEEP_RUNNING_UNTIL_FAILURE_NODE_H
