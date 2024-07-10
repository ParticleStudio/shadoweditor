#include "behaviortree/control/sequence_node.h"

namespace behaviortree {
SequenceNode::SequenceNode(const std::string& refName, bool refMakeAsync)
    : ControlNode::ControlNode(refName, {}), m_CurrentChildIdx(0), m_AllSkipped(true), m_Asynch(refMakeAsync) {
    if(m_Asynch) {
        SetRegistrationId("AsyncSequence");
    } else {
        SetRegistrationId("Sequence");
    }
}

void SequenceNode::Halt() {
    m_CurrentChildIdx = 0;
    ControlNode::Halt();
}

NodeStatus SequenceNode::Tick() {
    const size_t children_count = m_ChildrenNodesVec.size();

    if(GetNodeStatus() == NodeStatus::IDLE) {
        m_AllSkipped = true;
    }

    SetNodeStatus(NodeStatus::RUNNING);

    while(m_CurrentChildIdx < children_count) {
        TreeNode* ptrCurrentChildNode = m_ChildrenNodesVec[m_CurrentChildIdx];

        auto preNodeStatus = ptrCurrentChildNode->GetNodeStatus();
        const NodeStatus childNodeStatus = ptrCurrentChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        m_AllSkipped &= (childNodeStatus == NodeStatus::SKIPPED);

        switch(childNodeStatus) {
            case NodeStatus::RUNNING: {
                return NodeStatus::RUNNING;
            }
            case NodeStatus::FAILURE: {
                // Reset on failure
                ResetChildren();
                m_CurrentChildIdx = 0;
                return childNodeStatus;
            }
            case NodeStatus::SUCCESS: {
                m_CurrentChildIdx++;
                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(m_Asynch && RequiresWakeUp() && preNodeStatus == NodeStatus::IDLE &&
                   m_CurrentChildIdx < children_count) {
                    EmitWakeUpSignal();
                    return NodeStatus::RUNNING;
                }
            } break;
            case NodeStatus::SKIPPED: {
                // It was requested to skip this node
                m_CurrentChildIdx++;
            } break;
            case NodeStatus::IDLE: {
                throw LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            }
            default:{

            } break;
        }// end switch
    }// end while loop

    // The entire while loop completed. This means that all the children returned SUCCESS.
    if(m_CurrentChildIdx == children_count) {
        ResetChildren();
        m_CurrentChildIdx = 0;
    }
    // Skip if ALL the nodes have been skipped
    return m_AllSkipped ? NodeStatus::SKIPPED : NodeStatus::SUCCESS;
}

}// namespace behaviortree
