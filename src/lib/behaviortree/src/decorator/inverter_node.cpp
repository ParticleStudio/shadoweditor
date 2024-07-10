#include "behaviortree/decorator/inverter_node.h"

namespace behaviortree {
InverterNode::InverterNode(const std::string &refName): DecoratorNode(refName, {}) {
    SetRegistrationId("Inverter");
}

NodeStatus InverterNode::Tick() {
    SetNodeStatus(NodeStatus::RUNNING);
    const NodeStatus child_status = m_ChildNode->ExecuteTick();

    switch(child_status) {
        case NodeStatus::SUCCESS: {
            ResetChild();
            return NodeStatus::FAILURE;
        }
        case NodeStatus::FAILURE: {
            ResetChild();
            return NodeStatus::SUCCESS;
        }
        case NodeStatus::RUNNING:
        case NodeStatus::SKIPPED: {
            return child_status;
        }

        case NodeStatus::IDLE: {
            throw LogicError(
                    "[", GetNodeName(), "]: A children should not return IDLE"
            );
        }
    }
    return GetNodeStatus();
}

}// namespace behaviortree
