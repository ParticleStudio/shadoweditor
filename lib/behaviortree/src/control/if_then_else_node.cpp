#include "behaviortree/control/if_then_else_node.hpp"

namespace behaviortree {
IfThenElseNode::IfThenElseNode(const std::string &rName): ControlNode::ControlNode(rName, {}), m_childNodeIdx(0) {
    SetRegistrationId("IfThenElse");
}

void IfThenElseNode::Halt() {
    m_childNodeIdx = 0;
    ControlNode::Halt();
}

NodeStatus IfThenElseNode::Tick() {
    const size_t childrenNum = m_childrenNodeVec.size();

    if(childrenNum != 2 and childrenNum != 3) {
        throw std::logic_error("IfThenElseNode must have either 2 or 3 children");
    }

    SetNodeStatus(NodeStatus::Running);

    if(m_childNodeIdx == 0) {
        NodeStatus conditionStatus = m_childrenNodeVec[0]->ExecuteTick();

        if(conditionStatus == NodeStatus::Running) {
            return conditionStatus;
        } else if(conditionStatus == NodeStatus::Success) {
            m_childNodeIdx = 1;
        } else if(conditionStatus == NodeStatus::Failure) {
            if(childrenNum == 3) {
                m_childNodeIdx = 2;
            } else {
                return conditionStatus;
            }
        }
    }
    // not an else
    if(m_childNodeIdx > 0) {
        NodeStatus status = m_childrenNodeVec[m_childNodeIdx]->ExecuteTick();
        if(status == NodeStatus::Running) {
            return NodeStatus::Running;
        } else {
            ResetChildren();
            m_childNodeIdx = 0;
            return status;
        }
    }

    throw std::logic_error("Something unexpected happened in IfThenElseNode");
}

}// namespace behaviortree
