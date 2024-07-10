#ifndef BEHAVIORTREE_DECORATOR_NODE_H
#define BEHAVIORTREE_DECORATOR_NODE_H

#include "behaviortree/tree_node.h"

namespace behaviortree {
class DecoratorNode: public TreeNode {
 protected:
    TreeNode *m_ChildNode;

 public:
    DecoratorNode(const std::string &refName, const NodeConfig &refConfig);

    virtual ~DecoratorNode() override = default;

    void SetChild(TreeNode *ptrChild);

    const TreeNode *GetChild() const;

    TreeNode *GetChild();

    /// The method used to interrupt the execution of this node
    virtual void Halt() override;

    /// Same as resetChild()
    void HaltChild();

    virtual NodeType Type() const override {
        return NodeType::DECORATOR;
    }

    NodeStatus ExecuteTick() override;

    /// Set the status of the GetChild to IDLE.
    /// also send a halt() signal to a RUNNING GetChild
    void ResetChild();
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
    SimpleDecoratorNode(
            const std::string &refName, TickFunctor tickFunctor,
            const NodeConfig &refConfig
    );

    ~SimpleDecoratorNode() override = default;

 protected:
    virtual NodeStatus Tick() override;

    TickFunctor m_TickFunctor;
};
}// namespace behaviortree

#endif// BEHAVIORTREE_DECORATOR_NODE_H
