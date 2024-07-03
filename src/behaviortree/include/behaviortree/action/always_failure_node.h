#ifndef BEHAVIORTREE_ALWAYS_FAILURE_NODE_H
#define BEHAVIORTREE_ALWAYS_FAILURE_NODE_H

#include "behaviortree/action_node.h"

namespace BT {
/**
 * Simple actions that always returns FAILURE.
 */
class AlwaysFailureNode: public SyncActionNode {
 public:
    AlwaysFailureNode(const std::string& refName): SyncActionNode(refName, {}) {
        SetRegistrationID("AlwaysFailure");
    }

 private:
    virtual BT::NodeStatus Tick() override {
        return NodeStatus::FAILURE;
    }
};
}// namespace BT

#endif// BEHAVIORTREE_ALWAYS_FAILURE_NODE_H
