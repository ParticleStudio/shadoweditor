#ifndef BEHAVIORTREE_CONDITION_NODE_H
#define BEHAVIORTREE_CONDITION_NODE_H

#include "leaf_node.h"

namespace behaviortree {
class ConditionNode: public LeafNode {
 public:
    ConditionNode(const std::string& refName, const NodeConfig& refConfig);

    virtual ~ConditionNode() override = default;

    // Do nothing
    virtual void Halt() override final {
        ResetNodeStatus();
    }

    virtual NodeType Type() const override final {
        return NodeType::CONDITION;
    }
};

/**
 * @brief The SimpleConditionNode provides an easy to use ConditionNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(void)
 *
 * This avoids the hassle of inheriting from a ActionNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 */
class SimpleConditionNode: public ConditionNode {
 public:
    using TickFunctor = std::function<NodeStatus(TreeNode&)>;

    // You must provide the function to call when tick() is invoked
    SimpleConditionNode(const std::string& refName, TickFunctor tickFunctor,
                        const NodeConfig& refConfig);

    ~SimpleConditionNode() override = default;

 protected:
    virtual NodeStatus Tick() override;

    TickFunctor m_TickFunctor;
};
}// namespace behaviortree

#endif// BEHAVIORTREE_CONDITION_NODE_H
