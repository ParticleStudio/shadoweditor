#include "behaviortree/control/while_do_else_node.h"

namespace behaviortree {
WhileDoElseNode::WhileDoElseNode(const std::string &name): ControlNode::ControlNode(name, {}) {
    SetRegistrationId("WhileDoElse");
}

void WhileDoElseNode::Halt() {
    ControlNode::Halt();
}

NodeStatus WhileDoElseNode::Tick() {
    const size_t children_count = m_ChildrenNodeVec.size();

    if(children_count != 2 && children_count != 3) {
        throw std::logic_error(
                "WhileDoElseNode must have either 2 or 3 children"
        );
    }

    SetNodeStatus(NodeStatus::Running);

    NodeStatus condition_status = m_ChildrenNodeVec[0]->ExecuteTick();

    if(condition_status == NodeStatus::Running) {
        return condition_status;
    }

    NodeStatus status = NodeStatus::Idle;

    if(condition_status == NodeStatus::Success) {
        if(children_count == 3) {
            HaltChild(2);
        }
        status = m_ChildrenNodeVec[1]->ExecuteTick();
    } else if(condition_status == NodeStatus::Failure) {
        if(children_count == 3) {
            HaltChild(1);
            status = m_ChildrenNodeVec[2]->ExecuteTick();
        } else if(children_count == 2) {
            status = NodeStatus::Failure;
        }
    }

    if(status == NodeStatus::Running) {
        return NodeStatus::Running;
    } else {
        ResetChildren();
        return status;
    }
}

}// namespace behaviortree
