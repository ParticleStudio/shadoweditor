#include "behaviortree/control/sequence_with_memory_node.h"

namespace behaviortree {
SequenceWithMemory::SequenceWithMemory(const std::string& refName): ControlNode::ControlNode(refName, {}),
                                                                    m_CurrentChildIdx(0) {
    SetRegistrationId("SequenceWithMemory");
}

NodeStatus SequenceWithMemory::Tick() {
    const size_t childrenCount = m_ChildrenNodesVec.size();

    if(GetNodeStatus() == NodeStatus::IDLE) {
        m_AllSkipped = true;
    }
    SetNodeStatus(NodeStatus::RUNNING);

    while(m_CurrentChildIdx < childrenCount) {
        TreeNode* ptrCurrentChildNode = m_ChildrenNodesVec[m_CurrentChildIdx];

        auto preNodeStatus = ptrCurrentChildNode->GetNodeStatus();
        const NodeStatus childNodetatus = ptrCurrentChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        m_AllSkipped &= (childNodetatus == NodeStatus::SKIPPED);

        switch(childNodetatus) {
            case NodeStatus::RUNNING: {
                return childNodetatus;
            }
            case NodeStatus::FAILURE: {
                // DO NOT reset current_child_idx_ on failure
                for(size_t i = m_CurrentChildIdx; i < GetChildrenCount(); i++) {
                    HaltChild(i);
                }

                return childNodetatus;
            }
            case NodeStatus::SUCCESS: {
                m_CurrentChildIdx++;
                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(RequiresWakeUp() && preNodeStatus == NodeStatus::IDLE &&
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

    // The entire while loop completed. This means that all the children returned SUCCESS.
    if(m_CurrentChildIdx == childrenCount) {
        ResetChildren();
        m_CurrentChildIdx = 0;
    }
    // Skip if ALL the nodes have been skipped
    return m_AllSkipped ? NodeStatus::SKIPPED : NodeStatus::SUCCESS;
}

void SequenceWithMemory::Halt() {
    // should we add this line of code or not?
    // current_child_idx_ = 0;
    ControlNode::Halt();
}

}// namespace behaviortree
