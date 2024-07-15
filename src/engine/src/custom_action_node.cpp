#include "custom_action_node.h"

namespace engine {
CustomSyncActionNode::CustomSyncActionNode(const std::string &refName, const behaviortree::NodeConfig &refConfig)
    : behaviortree::SyncActionNode(refName, refConfig) {
}

behaviortree::NodeStatus CustomSyncActionNode::ExecuteTick() {
    return behaviortree::NodeStatus::SUCCESS;
}
}// namespace engine