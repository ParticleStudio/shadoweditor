#ifndef BEHAVIORTREE_WHILE_DO_ELSE_NODE_H
#define BEHAVIORTREE_WHILE_DO_ELSE_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief WhileDoElse must have exactly 2 or 3 Children.
 * It is a REACTIVE node of IfThenElseNode.
 *
 * The first GetChild is the "statement" that is executed at each tick
 *
 * If result is SUCCESS, the second GetChild is executed.
 *
 * If result is FAILURE, the third GetChild is executed.
 *
 * If the 2nd or 3d GetChild is RUNNING and the statement changes,
 * the RUNNING GetChild will be stopped before starting the sibling.
 *
 */
class WhileDoElseNode: public ControlNode {
 public:
    WhileDoElseNode(const std::string& refName);

    virtual ~WhileDoElseNode() override = default;

    virtual void Halt() override;

 private:
    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_WHILE_DO_ELSE_NODE_H
