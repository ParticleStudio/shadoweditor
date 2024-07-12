#ifndef BEHAVIORTREE_ALWAYS_SUCCESS_NODE_H
#define BEHAVIORTREE_ALWAYS_SUCCESS_NODE_H

#include "behaviortree/action_node.h"

namespace behaviortree {
/**
 * Simple actions that always returns SUCCESS.
 */
class AlwaysSuccessNode: public SyncActionNode {
 public:
    AlwaysSuccessNode(const std::string &refName): SyncActionNode(refName, {}) {
        SetRegistrationId("AlwaysSuccess");
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        return NodeStatus::SUCCESS;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_ALWAYS_SUCCESS_NODE_H
