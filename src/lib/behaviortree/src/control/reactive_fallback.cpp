#include "behaviortree/control/reactive_fallback.h"

namespace behaviortree {

bool ReactiveFallback::m_ThrowIfMultipleRunning = false;

void ReactiveFallback::EnableException(bool enable) {
    ReactiveFallback::m_ThrowIfMultipleRunning = enable;
}

NodeStatus ReactiveFallback::Tick() {
    bool allSkipped = true;
    if(GetNodeStatus() == NodeStatus::IDLE) {
        m_RunningChild = -1;
    }
    SetNodeStatus(NodeStatus::RUNNING);

    for(int32_t index = 0; index < GetChildrenCount(); index++) {
        TreeNode* ptrCurrentChildNode = m_ChildrenNodesVec[index];
        const NodeStatus childNodeStatus = ptrCurrentChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        allSkipped &= (childNodeStatus == NodeStatus::SKIPPED);

        switch(childNodeStatus) {
            case NodeStatus::RUNNING: {
                // reset the previous children, to make sure that they are
                // in IDLE state the next time we tick them
                for(size_t i = 0; i < GetChildrenCount(); i++) {
                    if(i != index) {
                        HaltChild(i);
                    }
                }
                if(m_RunningChild == -1) {
                    m_RunningChild = int(index);
                } else if(m_ThrowIfMultipleRunning && m_RunningChild != int(index)) {
                    throw LogicError(
                            "[ReactiveFallback]: only a single child can return RUNNING.\n"
                            "This throw can be disabled with "
                            "ReactiveFallback::EnableException(false)"
                    );
                }
                return NodeStatus::RUNNING;
            }

            case NodeStatus::FAILURE:
                break;

            case NodeStatus::SUCCESS: {
                ResetChildren();
                return NodeStatus::SUCCESS;
            }

            case NodeStatus::SKIPPED: {
                // to allow it to be skipped again, we must reset the node
                HaltChild(index);
            } break;

            case NodeStatus::IDLE: {
                throw LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            }
        }// end switch
    }//end for

    ResetChildren();

    // Skip if ALL the nodes have been skipped
    return allSkipped ? NodeStatus::SKIPPED : NodeStatus::FAILURE;
}

void ReactiveFallback::Halt() {
    m_RunningChild = -1;
    ControlNode::Halt();
}

}// namespace behaviortree
