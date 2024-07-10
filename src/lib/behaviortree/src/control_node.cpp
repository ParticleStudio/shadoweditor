#include "behaviortree/control_node.h"

namespace behaviortree {
ControlNode::ControlNode(
        const std::string& refName, const NodeConfig& refConfig
): TreeNode::TreeNode(refName, refConfig) {}

void ControlNode::AddChild(TreeNode* ptrChild) {
    m_ChildrenNodesVec.push_back(ptrChild);
}

size_t ControlNode::GetChildrenCount() const {
    return m_ChildrenNodesVec.size();
}

void ControlNode::Halt() {
    ResetChildren();
    ResetNodeStatus();// might be redundant
}

void ControlNode::ResetChildren() {
    for(auto ptrChild: m_ChildrenNodesVec) {
        if(ptrChild->GetNodeStatus() == NodeStatus::RUNNING) {
            ptrChild->HaltNode();
        }
        ptrChild->ResetNodeStatus();
    }
}

const std::vector<TreeNode*>& ControlNode::Children() const {
    return m_ChildrenNodesVec;
}

void ControlNode::HaltChild(size_t i) {
    auto child = m_ChildrenNodesVec[i];
    if(child->GetNodeStatus() == NodeStatus::RUNNING) {
        child->HaltNode();
    }
    child->ResetNodeStatus();
}

void ControlNode::HaltChildren() {
    for(size_t i = 0; i < m_ChildrenNodesVec.size(); i++) {
        HaltChild(i);
    }
}

void ControlNode::HaltChildren(size_t first) {
    for(size_t i = first; i < m_ChildrenNodesVec.size(); i++) {
        HaltChild(i);
    }
}

}// namespace behaviortree
