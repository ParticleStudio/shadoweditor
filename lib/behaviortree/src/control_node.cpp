#include "behaviortree/control_node.h"

namespace behaviortree {
ControlNode::ControlNode(
        const std::string &refName, const NodeConfig &refConfig
): TreeNode::TreeNode(refName, refConfig) {}

void ControlNode::AddChildNode(TreeNode *ptrChildNode) {
    m_ChildrenNodeVec.push_back(ptrChildNode);
}

size_t ControlNode::GetChildrenNum() const {
    return m_ChildrenNodeVec.size();
}

void ControlNode::Halt() {
    ResetChildren();
    ResetNodeStatus();// might be redundant
}

void ControlNode::ResetChildren() {
    for(auto *ptrChild: m_ChildrenNodeVec) {
        if(ptrChild->GetNodeStatus() == NodeStatus::Running) {
            ptrChild->HaltNode();
        }
        ptrChild->ResetNodeStatus();
    }
}

const std::vector<TreeNode *> &ControlNode::GetChildrenNode() const {
    return m_ChildrenNodeVec;
}

void ControlNode::HaltChild(size_t i) {
    auto ptrChildNode = m_ChildrenNodeVec[i];
    if(ptrChildNode->GetNodeStatus() == NodeStatus::Running) {
        ptrChildNode->HaltNode();
    }
    ptrChildNode->ResetNodeStatus();
}

void ControlNode::HaltChildren() {
    for(size_t i = 0; i < m_ChildrenNodeVec.size(); i++) {
        HaltChild(i);
    }
}

void ControlNode::HaltChildren(size_t beginIndex) {
    for(size_t i = beginIndex; i < m_ChildrenNodeVec.size(); i++) {
        HaltChild(i);
    }
}

}// namespace behaviortree
