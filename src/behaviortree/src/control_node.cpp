#include "behaviortree/control_node.h"

namespace behaviortree {
ControlNode::ControlNode(const std::string& refName, const NodeConfig& refConfig)
    : TreeNode::TreeNode(refName, refConfig) {}

void ControlNode::AddChild(TreeNode* ptrChild) {
    m_ChildrenNodesVec.push_back(ptrChild);
}

size_t ControlNode::ChildrenCount() const {
    return m_ChildrenNodesVec.size();
}

void ControlNode::halt() {
    resetChildren();
    resetStatus();// might be redundant
}

void ControlNode::resetChildren() {
    for(auto ptrChild: m_ChildrenNodesVec) {
        if(ptrChild->nodeStatus() == NodeStatus::RUNNING) {
            ptrChild->haltNode();
        }
        ptrChild->resetStatus();
    }
}

const std::vector<TreeNode*>& ControlNode::Children() const {
    return m_ChildrenNodesVec;
}

void ControlNode::haltChild(size_t i) {
    auto child = m_ChildrenNodesVec[i];
    if(child->nodeStatus() == NodeStatus::RUNNING) {
        child->haltNode();
    }
    child->resetStatus();
}

void ControlNode::haltChildren() {
    for(size_t i = 0; i < m_ChildrenNodesVec.size(); i++) {
        haltChild(i);
    }
}

void ControlNode::haltChildren(size_t first) {
    for(size_t i = first; i < m_ChildrenNodesVec.size(); i++) {
        haltChild(i);
    }
}

}// namespace behaviortree
