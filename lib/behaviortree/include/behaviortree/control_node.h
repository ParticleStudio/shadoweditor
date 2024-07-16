#ifndef BEHAVIORTREE_CONTROL_NODE_H
#define BEHAVIORTREE_CONTROL_NODE_H

#include <vector>

#include "behaviortree/tree_node.h"

namespace behaviortree {
class ControlNode: public TreeNode {
 protected:
    std::vector<TreeNode *> m_ChildrenNodeVec;

 public:
    ControlNode(const std::string &refName, const NodeConfig &refConfig);

    virtual ~ControlNode() override = default;

    /// The method used to Add nodes to the GetChildrenNode vector
    void AddChildNode(TreeNode *ptrChildNode);

    size_t GetChildrenNum() const;

    const std::vector<TreeNode *> &GetChildrenNode() const;

    const TreeNode *Child(size_t index) const {
        return GetChildrenNode().at(index);
    }

    virtual void Halt() override;

    /// same as resetChildren()
    void HaltChildren();

    [[deprecated(
            "deprecated: please use explicitly HaltChildren() or HaltChild(i)"
    )]] void
    HaltChildren(size_t beginIndex);

    void HaltChild(size_t i);

    virtual NodeType Type() const override final {
        return NodeType::Control;
    }

    /// Set the status of all GetChildrenNode to IDLE.
    /// also send a halt() signal to all RUNNING GetChildrenNode
    void ResetChildren();
};
}// namespace behaviortree

#endif// BEHAVIORTREE_CONTROL_NODE_H
