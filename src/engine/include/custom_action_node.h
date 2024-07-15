#ifndef ENGINE_CUSTOM_ACTION_NODE_H
#define ENGINE_CUSTOM_ACTION_NODE_H

#include "behaviortree/action_node.h"
#include "behaviortree/behaviortree.h"

namespace engine {

class CustomSyncActionNode: public behaviortree::SyncActionNode {
 public:
    CustomSyncActionNode(const std::string &refName, const behaviortree::NodeConfig &refConfig);
    behaviortree::NodeStatus ExecuteTick() override;
};

}// namespace engine

#endif//ENGINE_CUSTOM_ACTION_NODE_H
