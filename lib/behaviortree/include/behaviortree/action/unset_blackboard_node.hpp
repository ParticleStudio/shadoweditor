#ifndef BEHAVIORTREE_UNSET_BLACKBOARD_NODE_HPP
#define BEHAVIORTREE_UNSET_BLACKBOARD_NODE_HPP

#include "behaviortree/action_node.h"
#include "behaviortree/common.h"

namespace behaviortree {
/**
 * Action that removes an entry from the blackboard and return SUCCESS.
 */
class BEHAVIORTREE_API UnsetBlackboardNode: public SyncActionNode {
 public:
    UnsetBlackboardNode(const std::string &rName, const NodeConfig &rConfig): SyncActionNode(rName, rConfig) {
        SetRegistrationId("UnsetBlackboard");
    }

    static PortMap ProvidedPorts() {
        return {InputPort<std::string>("key", "Key of the entry to remove")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        std::string key;
        if(!GetInput("key", key)) {
            throw util::RuntimeError("missing input port [key]");
        }
        GetConfig().pBlackboard->Unset(key);
        return NodeStatus::Success;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_UNSET_BLACKBOARD_NODE_HPP
