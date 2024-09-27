#ifndef BEHAVIORTREE_ALWAYS_SUCCESS_NODE_HPP
#define BEHAVIORTREE_ALWAYS_SUCCESS_NODE_HPP

#include "behaviortree/action_node.h"
#include "behaviortree/common.h"

namespace behaviortree {
/**
 * Simple actions that always returns SUCCESS.
 */
class BEHAVIORTREE_API AlwaysSuccessNode: public SyncActionNode {
 public:
    AlwaysSuccessNode(const std::string &rName): SyncActionNode(rName, {}) {
        SetRegistrationId("AlwaysSuccess");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        return NodeStatus::Success;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_ALWAYS_SUCCESS_NODE_HPP