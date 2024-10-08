import common.exception;

#include "behaviortree/decorator_node.h"

namespace behaviortree {
DecoratorNode::DecoratorNode(const std::string &rName, const NodeConfig &rConfig): TreeNode::TreeNode(rName, rConfig), m_childNode(nullptr) {}

void DecoratorNode::SetChildNode(TreeNode *pChildNode) {
    if(m_childNode != nullptr) {
        throw util::Exception("Decorator [", GetNodeName(), "] has already a GetChildNode assigned");
    }

    m_childNode = pChildNode;
}

void DecoratorNode::Halt() {
    ResetChildNode();
    ResetNodeStatus();// might be redundant
}

const TreeNode *DecoratorNode::GetChildNode() const {
    return m_childNode;
}

TreeNode *DecoratorNode::GetChildNode() {
    return m_childNode;
}

void DecoratorNode::HaltChildNode() {
    ResetChildNode();
}

void DecoratorNode::ResetChildNode() {
    if(m_childNode == nullptr) {
        return;
    }
    if(m_childNode->GetNodeStatus() == NodeStatus::Running) {
        m_childNode->HaltNode();
    }
    m_childNode->ResetNodeStatus();
}

SimpleDecoratorNode::SimpleDecoratorNode(const std::string &refName, TickFunctor tickFunctor, const NodeConfig &refConfig): DecoratorNode(refName, refConfig), m_tickFunctor(std::move(tickFunctor)) {
}

NodeStatus SimpleDecoratorNode::Tick() {
    return m_tickFunctor(GetChildNode()->ExecuteTick(), *this);
}

NodeStatus DecoratorNode::ExecuteTick() {
    NodeStatus nodeStatus = TreeNode::ExecuteTick();
    NodeStatus childNodeStatus = GetChildNode()->GetNodeStatus();
    if(childNodeStatus == NodeStatus::Success or childNodeStatus == NodeStatus::Failure) {
        GetChildNode()->ResetNodeStatus();
    }
    return nodeStatus;
}

}// namespace behaviortree
