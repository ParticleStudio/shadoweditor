#include "behaviortree/control/while_do_else_node.h"

namespace behaviortree {
WhileDoElseNode::WhileDoElseNode(const std::string& name): ControlNode::ControlNode(name, {}) {
    SetRegistrationId("WhileDoElse");
}

void WhileDoElseNode::Halt() {
    ControlNode::Halt();
}

NodeStatus WhileDoElseNode::Tick() {
    const size_t children_count = m_ChildrenNodesVec.size();

    if(children_count != 2 && children_count != 3) {
        throw std::logic_error(
                "WhileDoElseNode must have either 2 or 3 children"
        );
    }

    SetNodeStatus(NodeStatus::RUNNING);

    NodeStatus condition_status = m_ChildrenNodesVec[0]->ExecuteTick();

    if(condition_status == NodeStatus::RUNNING) {
        return condition_status;
    }

    NodeStatus status = NodeStatus::IDLE;

    if(condition_status == NodeStatus::SUCCESS) {
        if(children_count == 3) {
            HaltChild(2);
        }
        status = m_ChildrenNodesVec[1]->ExecuteTick();
    } else if(condition_status == NodeStatus::FAILURE) {
        if(children_count == 3) {
            HaltChild(1);
            status = m_ChildrenNodesVec[2]->ExecuteTick();
        } else if(children_count == 2) {
            status = NodeStatus::FAILURE;
        }
    }

    if(status == NodeStatus::RUNNING) {
        return NodeStatus::RUNNING;
    } else {
        ResetChildren();
        return status;
    }
}

}// namespace behaviortree
