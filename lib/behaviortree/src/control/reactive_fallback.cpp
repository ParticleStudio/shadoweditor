module behaviortree.reactive_fallback;

namespace behaviortree {

bool ReactiveFallback::m_throwIfMultipleRunning = false;

void ReactiveFallback::EnableException(bool enable) {
    ReactiveFallback::m_throwIfMultipleRunning = enable;
}

NodeStatus ReactiveFallback::Tick() {
    bool allSkipped = true;
    if(GetNodeStatus() == NodeStatus::Idle) {
        m_runningChild = -1;
    }
    SetNodeStatus(NodeStatus::Running);

    for(int32_t index = 0; index < GetChildrenNum(); index++) {
        TreeNode *pCurChildNode = m_childrenNodeVec[index];
        const NodeStatus childNodeStatus = pCurChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        allSkipped &= (childNodeStatus == NodeStatus::Skipped);

        switch(childNodeStatus) {
            case NodeStatus::Running: {
                // reset the previous children, to make sure that they are
                // in IDLE state the next time we tick them
                for(size_t i = 0; i < GetChildrenNum(); i++) {
                    if(i != index) {
                        HaltChild(i);
                    }
                }
                if(m_runningChild == -1) {
                    m_runningChild = int(index);
                } else if(m_throwIfMultipleRunning && m_runningChild != int(index)) {
                    throw util::LogicError(
                            "[ReactiveFallback]: only a single child can "
                            "return RUNNING.\n"
                            "This throw can be disabled with "
                            "ReactiveFallback::EnableException(false)"
                    );
                }
                return NodeStatus::Running;
            } break;
            case NodeStatus::Failure: {
            } break;
            case NodeStatus::Success: {
                ResetChildren();
                return NodeStatus::Success;
            } break;
            case NodeStatus::Skipped: {
                // to allow it to be skipped again, we must reset the node
                HaltChild(index);
            } break;
            case NodeStatus::Idle: {
                throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            } break;
            default: {

            } break;
        }// end switch
    }//end for

    ResetChildren();

    // Skip if ALL the nodes have been skipped
    return allSkipped ? NodeStatus::Skipped : NodeStatus::Failure;
}

void ReactiveFallback::Halt() {
    m_runningChild = -1;
    ControlNode::Halt();
}

}// namespace behaviortree

// module behaviortree.reactive_fallback;
