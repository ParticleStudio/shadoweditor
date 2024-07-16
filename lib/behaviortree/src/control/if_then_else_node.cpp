#include "behaviortree/control/if_then_else_node.h"

namespace behaviortree {
IfThenElseNode::IfThenElseNode(const std::string &refName): ControlNode::ControlNode(refName, {}),
                                                            m_ChildIdx(0) {
    SetRegistrationId("IfThenElse");
}

void IfThenElseNode::Halt() {
    m_ChildIdx = 0;
    ControlNode::Halt();
}

NodeStatus IfThenElseNode::Tick() {
    const size_t childrenCount = m_ChildrenNodeVec.size();

    if(childrenCount != 2 && childrenCount != 3) {
        throw std::logic_error("IfThenElseNode must have either 2 or 3 children"
        );
    }

    SetNodeStatus(NodeStatus::Running);

    if(m_ChildIdx == 0) {
        NodeStatus conditionStatus = m_ChildrenNodeVec[0]->ExecuteTick();

        if(conditionStatus == NodeStatus::Running) {
            return conditionStatus;
        } else if(conditionStatus == NodeStatus::Success) {
            m_ChildIdx = 1;
        } else if(conditionStatus == NodeStatus::Failure) {
            if(childrenCount == 3) {
                m_ChildIdx = 2;
            } else {
                return conditionStatus;
            }
        }
    }
    // not an else
    if(m_ChildIdx > 0) {
        NodeStatus status = m_ChildrenNodeVec[m_ChildIdx]->ExecuteTick();
        if(status == NodeStatus::Running) {
            return NodeStatus::Running;
        } else {
            ResetChildren();
            m_ChildIdx = 0;
            return status;
        }
    }

    throw std::logic_error("Something unexpected happened in IfThenElseNode");
}

}// namespace behaviortree
