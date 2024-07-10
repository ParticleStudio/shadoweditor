#include "behaviortree/decorator_node.h"

namespace behaviortree {
DecoratorNode::DecoratorNode(
        const std::string& refName, const NodeConfig& refConfig
): TreeNode::TreeNode(refName, refConfig),
   m_ChildNode(nullptr) {}

void DecoratorNode::SetChild(TreeNode* ptrChild) {
    if(m_ChildNode != nullptr) {
        throw BehaviorTreeException(
                "Decorator [", GetNodeName(),
                "] has already a GetChild assigned"
        );
    }

    m_ChildNode = ptrChild;
}

void DecoratorNode::Halt() {
    ResetChild();
    ResetNodeStatus();// might be redundant
}

const TreeNode* DecoratorNode::GetChild() const {
    return m_ChildNode;
}

TreeNode* DecoratorNode::GetChild() {
    return m_ChildNode;
}

void DecoratorNode::HaltChild() {
    ResetChild();
}

void DecoratorNode::ResetChild() {
    if(m_ChildNode == nullptr) {
        return;
    }
    if(m_ChildNode->GetNodeStatus() == NodeStatus::RUNNING) {
        m_ChildNode->HaltNode();
    }
    m_ChildNode->ResetNodeStatus();
}

SimpleDecoratorNode::SimpleDecoratorNode(
        const std::string& refName, TickFunctor tickFunctor,
        const NodeConfig& refConfig
): DecoratorNode(refName, refConfig),
   m_TickFunctor(std::move(tickFunctor)) {}

NodeStatus SimpleDecoratorNode::Tick() {
    return m_TickFunctor(GetChild()->ExecuteTick(), *this);
}

NodeStatus DecoratorNode::ExecuteTick() {
    NodeStatus nodeStatus = TreeNode::ExecuteTick();
    NodeStatus childNodeStatus = GetChild()->GetNodeStatus();
    if(childNodeStatus == NodeStatus::SUCCESS ||
       childNodeStatus == NodeStatus::FAILURE) {
        GetChild()->ResetNodeStatus();
    }
    return nodeStatus;
}

}// namespace behaviortree
