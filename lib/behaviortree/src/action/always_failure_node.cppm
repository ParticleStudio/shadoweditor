module;

export module behaviortree.always_failure_node;

#include "behaviortree/action_node.h"
#include "behaviortree/common.h"

namespace behaviortree {
/**
 * Simple actions that always returns FAILURE.
 */
export class BEHAVIORTREE_API AlwaysFailureNode: public SyncActionNode {
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

// module behaviortree.always_failure_node;
// module;
