module behaviortree.while_do_else_node;

namespace behaviortree {
WhileDoElseNode::WhileDoElseNode(const std::string &rName): ControlNode::ControlNode(rName, {}) {
    SetRegistrationId("WhileDoElse");
}

void WhileDoElseNode::Halt() {
    ControlNode::Halt();
}

NodeStatus WhileDoElseNode::Tick() {
    const size_t childrenNum = m_childrenNodeVec.size();

    if(childrenNum != 2 && childrenNum != 3) {
        throw std::logic_error("WhileDoElseNode must have either 2 or 3 children");
    }

    SetNodeStatus(NodeStatus::Running);

    NodeStatus conditionStatus = m_childrenNodeVec[0]->ExecuteTick();

    if(conditionStatus == NodeStatus::Running) {
        return conditionStatus;
    }

    NodeStatus status = NodeStatus::Idle;

    if(conditionStatus == NodeStatus::Success) {
        if(childrenNum == 3) {
            HaltChild(2);
        }
        status = m_childrenNodeVec[1]->ExecuteTick();
    } else if(conditionStatus == NodeStatus::Failure) {
        if(childrenNum == 3) {
            HaltChild(1);
            status = m_childrenNodeVec[2]->ExecuteTick();
        } else if(childrenNum == 2) {
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

// module behaviortree.while_do_else_node;
