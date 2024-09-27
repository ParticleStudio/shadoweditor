#ifndef BEHAVIORTREE_ALWAYS_FAILURE_NODE_HPP
#define BEHAVIORTREE_ALWAYS_FAILURE_NODE_HPP

#include "behaviortree/action_node.h"
#include "behaviortree/common.h"

namespace behaviortree {
/**
 * Simple actions that always returns FAILURE.
 */
class BEHAVIORTREE_API AlwaysFailureNode: public SyncActionNode {
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

#endif// BEHAVIORTREE_ALWAYS_FAILURE_NODE_HPP
