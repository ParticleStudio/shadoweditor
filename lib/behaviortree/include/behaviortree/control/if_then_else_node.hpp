#ifndef BEHAVIORTREE_IF_THEN_ELSE_NODE_HPP
#define BEHAVIORTREE_IF_THEN_ELSE_NODE_HPP

#include "behaviortree/common.h"
#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief IfThenElseNode must have exactly 2 or 3 GetChildrenNode. This node is NOT reactive.
 *
 * The first GetChildNode is the "statement" of the if.
 *
 * If that return SUCCESS, then the second GetChildNode is executed.
 *
 * Instead, if it returned FAILURE, the third GetChildNode is executed.
 *
 * If you have only 2 GetChildrenNode, this node will return FAILURE whenever the
 * statement returns FAILURE.
 *
 * This is equivalent to Add AlwaysFailure as 3rd GetChildNode.
 *
 */
class BEHAVIORTREE_API IfThenElseNode: public ControlNode {
 public:
    IfThenElseNode(const std::string &rName);

    virtual ~IfThenElseNode() override = default;

    virtual void Halt() override;

 private:
    size_t m_childNodeIdx;

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_IF_THEN_ELSE_NODE_HPP
