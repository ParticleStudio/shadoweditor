#include "behaviortree/control/fallback_node.h"

namespace behaviortree {
FallbackNode::FallbackNode(const std::string &refName, bool makeAsynch): ControlNode::ControlNode(refName, {}),
                                                                         m_CurrentChildIdx(0),
                                                                         m_AllSkipped(true),
                                                                         m_Asynch(makeAsynch) {
    if(m_Asynch) {
        SetRegistrationId("AsyncFallback");
    } else {
        SetRegistrationId("Fallback");
    }
}

NodeStatus FallbackNode::Tick() {
    const size_t childrenCount = m_ChildrenNodesVec.size();

    if(GetNodeStatus() == NodeStatus::IDLE) {
        m_AllSkipped = true;
    }

    SetNodeStatus(NodeStatus::RUNNING);

    while(m_CurrentChildIdx < childrenCount) {
        TreeNode *ptrCurrentChildNode = m_ChildrenNodesVec[m_CurrentChildIdx];

        auto preNodeStatus = ptrCurrentChildNode->GetNodeStatus();
        const NodeStatus childNodeStatus = ptrCurrentChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        m_AllSkipped &= (childNodeStatus == NodeStatus::SKIPPED);

        switch(childNodeStatus) {
            case NodeStatus::RUNNING: {
                return childNodeStatus;
            }
            case NodeStatus::SUCCESS: {
                ResetChildren();
                m_CurrentChildIdx = 0;
                return childNodeStatus;
            }
            case NodeStatus::FAILURE: {
                m_CurrentChildIdx++;
                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(m_Asynch && RequiresWakeUp() &&
                   preNodeStatus == NodeStatus::IDLE &&
                   m_CurrentChildIdx < childrenCount) {
                    EmitWakeUpSignal();
                    return NodeStatus::RUNNING;
                }
            } break;
            case NodeStatus::SKIPPED: {
                // It was requested to skip this node
                m_CurrentChildIdx++;
            } break;
            case NodeStatus::IDLE: {
                throw LogicError(
                        "[", GetNodeName(),
                        "]: A children should not return IDLE"
                );
            }
        }// end switch
    }// end while loop

    // The entire while loop completed. This means that all the children returned FAILURE.
    if(m_CurrentChildIdx == childrenCount) {
        ResetChildren();
        m_CurrentChildIdx = 0;
    }

    // Skip if ALL the nodes have been skipped
    return m_AllSkipped ? NodeStatus::SKIPPED : NodeStatus::FAILURE;
}

void FallbackNode::Halt() {
    m_CurrentChildIdx = 0;
    ControlNode::Halt();
}

}// namespace behaviortree
