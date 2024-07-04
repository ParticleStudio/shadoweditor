#ifndef BEHAVIORTREE_UNSET_BLACKBOARD_NODE_H
#define BEHAVIORTREE_UNSET_BLACKBOARD_NODE_H

#include "behaviortree/action_node.h"

namespace behaviortree {
/**
 * Action that removes an entry from the blackboard and return SUCCESS.
 */
class UnsetBlackboardNode: public SyncActionNode {
 public:
    UnsetBlackboardNode(const std::string& refName, const NodeConfig& refConfig)
        : SyncActionNode(refName, refConfig) {
        SetRegistrationID("UnsetBlackboard");
    }

    static PortsList ProvidedPorts() {
        return {InputPort<std::string>("key", "Key of the entry to remove")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        std::string key;
        if(!GetInput("key", key)) {
            throw RuntimeError("missing input port [key]");
        }
        GetConfig().ptrBlackboard->Unset(key);
        return NodeStatus::SUCCESS;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_UNSET_BLACKBOARD_NODE_H
