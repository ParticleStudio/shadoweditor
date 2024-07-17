#include "behaviortree/control_node.h"

namespace behaviortree {
ControlNode::ControlNode(const std::string &rName, const NodeConfig &rConfig): TreeNode::TreeNode(rName, rConfig) {}

void ControlNode::AddChildNode(TreeNode *pChildNode) {
    m_childrenNodeVec.push_back(pChildNode);
}

size_t ControlNode::GetChildrenNum() const {
    return m_childrenNodeVec.size();
}

void ControlNode::Halt() {
    ResetChildren();
    ResetNodeStatus();// might be redundant
}

void ControlNode::ResetChildren() {
    for(auto *pChildNode: m_childrenNodeVec) {
        if(pChildNode->GetNodeStatus() == NodeStatus::Running) {
            pChildNode->HaltNode();
        }
        pChildNode->ResetNodeStatus();
    }
}

const std::vector<TreeNode *> &ControlNode::GetChildrenNode() const {
    return m_childrenNodeVec;
}

void ControlNode::HaltChild(size_t i) {
    auto pChildNode = m_childrenNodeVec[i];
    if(pChildNode->GetNodeStatus() == NodeStatus::Running) {
        pChildNode->HaltNode();
    }
    pChildNode->ResetNodeStatus();
}

void ControlNode::HaltChildren() {
    for(size_t i = 0; i < m_childrenNodeVec.size(); i++) {
        HaltChild(i);
    }
}

void ControlNode::HaltChildren(size_t beginIndex) {
    for(size_t i = beginIndex; i < m_childrenNodeVec.size(); i++) {
        HaltChild(i);
    }
}

}// namespace behaviortree
