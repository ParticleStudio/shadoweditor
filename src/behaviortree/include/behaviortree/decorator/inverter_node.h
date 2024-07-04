#ifndef BEHAVIORTREE_INVERTER_NODE_H
#define BEHAVIORTREE_INVERTER_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The InverterNode returns SUCCESS if GetChild fails
 * of FAILURE is GetChild succeeds.
 * RUNNING status is propagated
 */
class InverterNode: public DecoratorNode {
 public:
    InverterNode(const std::string& refName);

    virtual ~InverterNode() override = default;

 private:
    virtual behaviortree::NodeStatus Tick() override;
};
}// namespace behaviortree

#endif// BEHAVIORTREE_INVERTER_NODE_H
