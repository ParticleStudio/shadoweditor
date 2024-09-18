export module behaviortree.decorator_node;

import behaviortree.tree_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
export class BEHAVIORTREE_API DecoratorNode: public TreeNode {
 protected:
    TreeNode *m_childNode;

 public:
    DecoratorNode(const std::string &rName, const NodeConfig &rConfig);

    virtual ~DecoratorNode() override = default;

    void SetChildNode(TreeNode *pChildNode);

    const TreeNode *GetChildNode() const;

    TreeNode *GetChildNode();

    /// The method used to interrupt the execution of this node
    virtual void Halt() override;

    /// Same as resetChild()
    void HaltChildNode();

    virtual NodeType Type() const override {
        return NodeType::Decorator;
    }

    NodeStatus ExecuteTick() override;

    /// Set the status of the GetChildNode to IDLE.
    /// also send a halt() signal to a RUNNING GetChildNode
    void ResetChildNode();
};

/**
 * @brief The SimpleDecoratorNode provides an easy to use DecoratorNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(BT::NodeStatus child_status)
 *
 * This avoids the hassle of inheriting from a DecoratorNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 */
class SimpleDecoratorNode: public DecoratorNode {
 public:
    using TickFunctor = std::function<NodeStatus(NodeStatus, TreeNode &)>;

    // You must provide the function to call when tick() is invoked
    SimpleDecoratorNode(const std::string &refName, TickFunctor tickFunctor,const NodeConfig &refConfig);

    ~SimpleDecoratorNode() override = default;

 protected:
    virtual NodeStatus Tick() override;

    TickFunctor m_tickFunctor;
};
}// namespace behaviortree

// module behaviortree.decorator_node;
