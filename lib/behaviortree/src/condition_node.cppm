export module behaviortree.condition_node;

import <string>

import behaviortree.leaf_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
export class BEHAVIORTREE_API ConditionNode: public LeafNode {
 public:
    ConditionNode(const std::string &rName, const NodeConfig &rConfig);

    virtual ~ConditionNode() override = default;

    // Do nothing
    virtual void Halt() override final {
        ResetNodeStatus();
    }

    virtual NodeType Type() const override final {
        return NodeType::Condition;
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
export class BEHAVIORTREE_API SimpleConditionNode: public ConditionNode {
 public:
    using TickFunctor = std::function<NodeStatus(TreeNode &)>;

    // You must provide the function to call when tick() is invoked
    SimpleConditionNode(
            const std::string &rName, TickFunctor tickFunctor,
            const NodeConfig &rConfig
    );

    ~SimpleConditionNode() override = default;

 protected:
    virtual NodeStatus Tick() override;

    TickFunctor m_tickFunctor;
};
}// namespace behaviortree

// module behaviortree.condition_node;
