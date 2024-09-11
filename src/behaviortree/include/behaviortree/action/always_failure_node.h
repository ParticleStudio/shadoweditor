#ifndef BEHAVIORTREE_ALWAYS_FAILURE_NODE_H
#define BEHAVIORTREE_ALWAYS_FAILURE_NODE_H

#include "behaviortree/action_node.h"

namespace behaviortree {
/**
 * Simple actions that always returns FAILURE.
 */
class AlwaysFailureNode: public SyncActionNode {
 public:
    AlwaysFailureNode(const std::string &rName): SyncActionNode(rName, {}) {
        SetRegistrationId("AlwaysFailure");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        return NodeStatus::Failure;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_ALWAYS_FAILURE_NODE_H
