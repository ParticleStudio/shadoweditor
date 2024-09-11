#ifndef BEHAVIORTREE_INVERTER_NODE_H
#define BEHAVIORTREE_INVERTER_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The InverterNode returns SUCCESS if GetChildNode fails
 * of FAILURE is GetChildNode succeeds.
 * RUNNING status is propagated
 */
class InverterNode: public DecoratorNode {
 public:
    InverterNode(const std::string &rName);

    virtual ~InverterNode() override = default;

 private:
    virtual behaviortree::NodeStatus Tick() override;
};
}// namespace behaviortree

#endif// BEHAVIORTREE_INVERTER_NODE_H
