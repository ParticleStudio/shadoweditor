module;

export module behaviortree.always_success_node;

#include "behaviortree/action_node.h"
#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * Simple actions that always returns SUCCESS.
 */
export class BEHAVIORTREE_API AlwaysSuccessNode: public SyncActionNode {
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

// module behaviortree.always_success_node;
// module;
