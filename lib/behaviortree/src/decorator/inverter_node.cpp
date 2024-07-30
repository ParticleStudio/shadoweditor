#include "behaviortree/decorator/inverter_node.h"

namespace behaviortree {
InverterNode::InverterNode(const std::string &rName): DecoratorNode(rName, {}) {
    SetRegistrationId("Inverter");
}

NodeStatus InverterNode::Tick() {
    SetNodeStatus(NodeStatus::Running);
    const NodeStatus childNodeStatus = m_childNode->ExecuteTick();

    switch(childNodeStatus) {
        case NodeStatus::Success: {
            ResetChildNode();
            return NodeStatus::Failure;
        }
        case NodeStatus::Failure: {
            ResetChildNode();
            return NodeStatus::Success;
        }
        case NodeStatus::Running:
        case NodeStatus::Skipped: {
            return childNodeStatus;
        }

        case NodeStatus::Idle: {
            throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
        }
    }
    return GetNodeStatus();
}

}// namespace behaviortree
