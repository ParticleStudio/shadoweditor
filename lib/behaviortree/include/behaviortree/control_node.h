#ifndef BEHAVIORTREE_CONTROL_NODE_H
#define BEHAVIORTREE_CONTROL_NODE_H

#include <vector>

#include "behaviortree/tree_node.h"

namespace behaviortree {
class ControlNode: public TreeNode {
 protected:
    std::vector<TreeNode *> m_ChildrenNodesVec;

 public:
    ControlNode(const std::string &refName, const NodeConfig &refConfig);

    virtual ~ControlNode() override = default;

    /// The method used to Add nodes to the Children vector
    void AddChild(TreeNode *ptrChild);

    size_t GetChildrenCount() const;

    const std::vector<TreeNode *> &Children() const;

    const TreeNode *Child(size_t index) const {
        return Children().at(index);
    }

    virtual void Halt() override;

    /// same as resetChildren()
    void HaltChildren();

    [[deprecated(
            "deprecated: please use explicitly HaltChildren() or HaltChild(i)"
    )]] void
    HaltChildren(size_t first);

    void HaltChild(size_t i);

    virtual NodeType Type() const override final {
        return NodeType::CONTROL;
    }

    /// Set the status of all Children to IDLE.
    /// also send a halt() signal to all RUNNING Children
    void ResetChildren();
};
}// namespace behaviortree

#endif// BEHAVIORTREE_CONTROL_NODE_H
