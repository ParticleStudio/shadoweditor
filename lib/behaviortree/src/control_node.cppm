export module behaviortree.control_node;

import <vector>;

import behaviortree.tree_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
export class BEHAVIORTREE_API ControlNode: public TreeNode {
 protected:
    std::vector<TreeNode *> m_childrenNodeVec;

 public:
    ControlNode(const std::string &rName, const NodeConfig &rConfig);

    virtual ~ControlNode() override = default;

    /// The method used to Add nodes to the GetChildrenNode vector
    void AddChildNode(TreeNode *pChildNode);

    size_t GetChildrenNum() const;

    const std::vector<TreeNode *> &GetChildrenNode() const;

    const TreeNode *GetChild(size_t index) const {
        return GetChildrenNode().at(index);
    }

    virtual void Halt() override;

    /// same as resetChildren()
    void HaltChildren();

    void HaltChild(size_t i);

    virtual NodeType Type() const override final {
        return NodeType::Control;
    }

    /// Set the status of all GetChildrenNode to IDLE.
    /// also send a halt() signal to all RUNNING GetChildrenNode
    void ResetChildren();
};
}// namespace behaviortree

// module behaviortree.control_node;
