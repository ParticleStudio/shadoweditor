#include "behaviortree/control/if_then_else_node.h"

namespace behaviortree {
IfThenElseNode::IfThenElseNode(const std::string& refName): ControlNode::ControlNode(refName, {}),
                                                            m_ChildIdx(0) {
    SetRegistrationId("IfThenElse");
}

void IfThenElseNode::Halt() {
    m_ChildIdx = 0;
    ControlNode::Halt();
}

NodeStatus IfThenElseNode::Tick() {
    const size_t childrenCount = m_ChildrenNodesVec.size();

    if(childrenCount != 2 && childrenCount != 3) {
        throw std::logic_error("IfThenElseNode must have either 2 or 3 children"
        );
    }

    SetNodeStatus(NodeStatus::RUNNING);

    if(m_ChildIdx == 0) {
        NodeStatus conditionStatus = m_ChildrenNodesVec[0]->ExecuteTick();

        if(conditionStatus == NodeStatus::RUNNING) {
            return conditionStatus;
        } else if(conditionStatus == NodeStatus::SUCCESS) {
            m_ChildIdx = 1;
        } else if(conditionStatus == NodeStatus::FAILURE) {
            if(childrenCount == 3) {
                m_ChildIdx = 2;
            } else {
                return conditionStatus;
            }
        }
    }
    // not an else
    if(m_ChildIdx > 0) {
        NodeStatus status = m_ChildrenNodesVec[m_ChildIdx]->ExecuteTick();
        if(status == NodeStatus::RUNNING) {
            return NodeStatus::RUNNING;
        } else {
            ResetChildren();
            m_ChildIdx = 0;
            return status;
        }
    }

    throw std::logic_error("Something unexpected happened in IfThenElseNode");
}

}// namespace behaviortree
