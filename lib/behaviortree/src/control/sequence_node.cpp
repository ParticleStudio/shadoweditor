#include "behaviortree/control/sequence_node.hpp"

namespace behaviortree {
SequenceNode::SequenceNode(const std::string &rName, bool rMakeAsync): ControlNode::ControlNode(rName, {}),
                                                                       m_currentChildIdx(0),
                                                                       m_asynch(rMakeAsync) {
    if(m_asynch) {
        SetRegistrationId("AsyncSequence");
    } else {
        SetRegistrationId("Sequence");
    }
}

void SequenceNode::Halt() {
    m_currentChildIdx = 0;
    ControlNode::Halt();
}

NodeStatus SequenceNode::Tick() {
    const size_t childrenNum = m_childrenNodeVec.size();

    if(GetNodeStatus() == NodeStatus::Idle) {
        m_skippedNum = 0;
    }

    SetNodeStatus(NodeStatus::Running);

    while(m_currentChildIdx < childrenNum) {
        TreeNode *pCurrentChildNode = m_childrenNodeVec[m_currentChildIdx];

        auto preNodeStatus = pCurrentChildNode->GetNodeStatus();
        const NodeStatus childNodeStatus = pCurrentChildNode->ExecuteTick();

        switch(childNodeStatus) {
            case NodeStatus::Running: {
                return NodeStatus::Running;
            }
            case NodeStatus::Failure: {
                // Reset on failure
                ResetChildren();
                m_currentChildIdx = 0;
                return childNodeStatus;
            }
            case NodeStatus::Success: {
                m_currentChildIdx++;
                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(m_asynch && RequiresWakeUp() && preNodeStatus == NodeStatus::Idle && m_currentChildIdx < childrenNum) {
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

}// namespace behaviortree
