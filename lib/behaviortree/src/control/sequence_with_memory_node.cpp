module;

module behaviortree.sequence_with_memory_node;

namespace behaviortree {
SequenceWithMemory::SequenceWithMemory(const std::string &rName): ControlNode::ControlNode(rName, {}), m_currentChildIdx(0) {
    SetRegistrationId("SequenceWithMemory");
}

NodeStatus SequenceWithMemory::Tick() {
    const size_t childrenNum = m_childrenNodeVec.size();

    if(GetNodeStatus() == NodeStatus::Idle) {
        m_skippedNum = 0;
    }
    SetNodeStatus(NodeStatus::Running);

    while(m_currentChildIdx < childrenNum) {
        TreeNode *ptrCurrentChildNode = m_childrenNodeVec[m_currentChildIdx];

        auto preNodeStatus = ptrCurrentChildNode->GetNodeStatus();
        const NodeStatus childNodetatus = ptrCurrentChildNode->ExecuteTick();

        switch(childNodetatus) {
            case NodeStatus::Running: {
                return childNodetatus;
            } break;
            case NodeStatus::Failure: {
                // DO NOT reset current_child_idx_ on failure
                for(size_t i = m_currentChildIdx; i < GetChildrenNum(); i++) {
                    HaltChild(i);
                }
                return childNodetatus;
            } break;
            case NodeStatus::Success: {
                m_currentChildIdx++;
                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(RequiresWakeUp() && preNodeStatus == NodeStatus::Idle && m_currentChildIdx < childrenNum) {
                    EmitWakeUpSignal();
                    return NodeStatus::Running;
                }
            } break;
            case NodeStatus::Skipped: {
                // It was requested to skip this node
                m_currentChildIdx++;
                m_skippedNum++;
            } break;
            case NodeStatus::Idle: {
                throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            } break;
            default: {

            } break;
        }// end switch
    }// end while loop

    // The entire while loop completed. This means that all the children returned SUCCESS.
    if(m_currentChildIdx == childrenNum) {
        ResetChildren();
        m_currentChildIdx = 0;
    }
    // Skip if ALL the nodes have been skipped
    return m_skippedNum == childrenNum ? NodeStatus::Skipped : NodeStatus::Success;
}

void SequenceWithMemory::Halt() {
    // should we add this line of code or not?
    // current_child_idx_ = 0;
    ControlNode::Halt();
}

}// namespace behaviortree

// module behaviortree.sequence_with_memory_node;
// module;
