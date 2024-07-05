#ifndef BEHAVIORTREE_KEEP_RUNNING_UNTIL_FAILURE_NODE_H
#define BEHAVIORTREE_KEEP_RUNNING_UNTIL_FAILURE_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The KeepRunningUntilFailureNode returns always FAILURE or RUNNING.
 */
class KeepRunningUntilFailureNode: public DecoratorNode {
 public:
    KeepRunningUntilFailureNode(const std::string& refName): DecoratorNode(refName, {}) {
        SetRegistrationId("KeepRunningUntilFailure");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus KeepRunningUntilFailureNode::Tick() {
    SetNodeStatus(NodeStatus::RUNNING);

    const NodeStatus childState = m_ChildNode->ExecuteTick();

    switch(childState) {
        case NodeStatus::FAILURE: {
            ResetChild();
            return NodeStatus::FAILURE;
        }
        case NodeStatus::SUCCESS: {
            ResetChild();
            return NodeStatus::RUNNING;
        }
        case NodeStatus::RUNNING: {
            return NodeStatus::RUNNING;
        }

        default: {
            // TODO throw?
        }
    }
    return GetNodeStatus();
}
}// namespace behaviortree

#endif// BEHAVIORTREE_KEEP_RUNNING_UNTIL_FAILURE_NODE_H
