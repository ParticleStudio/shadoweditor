#ifndef BEHAVIORTREE_IF_THEN_ELSE_NODE_H
#define BEHAVIORTREE_IF_THEN_ELSE_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief IfThenElseNode must have exactly 2 or 3 Children. This node is NOT reactive.
 *
 * The first Child is the "statement" of the if.
 *
 * If that return SUCCESS, then the second Child is executed.
 *
 * Instead, if it returned FAILURE, the third Child is executed.
 *
 * If you have only 2 Children, this node will return FAILURE whenever the
 * statement returns FAILURE.
 *
 * This is equivalent to add AlwaysFailure as 3rd Child.
 *
 */
class IfThenElseNode: public ControlNode {
 public:
    IfThenElseNode(const std::string& refName);

    virtual ~IfThenElseNode() override = default;

    virtual void Halt() override;

 private:
    size_t m_ChildIdx;

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_IF_THEN_ELSE_NODE_H
