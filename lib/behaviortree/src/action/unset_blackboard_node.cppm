export module behaviortree.unset_blackboard_node;

import behaviortree.action_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * Action that removes an entry from the blackboard and return SUCCESS.
 */
export class BEHAVIORTREE_API UnsetBlackboardNode: public SyncActionNode {
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

// module behaviortree.unset_blackboard_node;
