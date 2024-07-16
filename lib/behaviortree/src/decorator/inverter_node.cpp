#include "behaviortree/decorator/inverter_node.h"

namespace behaviortree {
InverterNode::InverterNode(const std::string &refName): DecoratorNode(refName, {}) {
    SetRegistrationId("Inverter");
}

NodeStatus InverterNode::Tick() {
    SetNodeStatus(NodeStatus::Running);
    const NodeStatus child_status = m_ChildNode->ExecuteTick();

    switch(child_status) {
        case NodeStatus::Success: {
            ResetChild();
            return NodeStatus::Failure;
        }
        case NodeStatus::Failure: {
            ResetChild();
            return NodeStatus::Success;
        }
        case NodeStatus::Running:
        case NodeStatus::Skipped: {
            return child_status;
        }

        case NodeStatus::Idle: {
            throw LogicError(
                    "[", GetNodeName(), "]: A children should not return IDLE"
            );
        }
    }
    return GetNodeStatus();
}

}// namespace behaviortree
